(function initFantasyGameServicesModule() {
  function normalizeGameServiceError(error, fallbackMessage = 'Game service request failed.') {
    if (error && typeof error === 'object') {
      return {
        code: error.code ?? 'E_GAME_SERVICE',
        details: error.details || error.message || fallbackMessage,
        message: error.message || error.details || fallbackMessage,
        raw: error
      };
    }

    const message = String(error || fallbackMessage);
    return {
      code: 'E_GAME_SERVICE',
      details: message,
      message,
      raw: error
    };
  }

  function createGameServices(api = window.gameApi) {
    if (!api) {
      throw new Error('window.gameApi is unavailable.');
    }

    const invoke = async (method, request) => {
      const handler = api[method];
      if (typeof handler !== 'function') {
        throw normalizeGameServiceError(new Error('Missing transport method: ' + method));
      }

      try {
        if (typeof request === 'undefined') {
          return await handler();
        }
        return await handler(request);
      } catch (error) {
        throw normalizeGameServiceError(error);
      }
    };

    return Object.freeze({
      checkServices: () => invoke('checkServices'),
      bootstrapGame: () => invoke('bootstrapGame'),
      calculateDamage: (request) => invoke('calculateDamage', request),
      getSkillTree: (request) => invoke('getSkillTree', request),
      aiDecision: (request) => invoke('aiDecision', request),
      getGameRules: (request) => invoke('getGameRules', request),
      updateWorldState: (request) => invoke('updateWorldState', request),
      triggerEvent: (request) => invoke('triggerEvent', request),
      queryGameState: (request) => invoke('queryGameState', request)
    });
  }

  const defaultServices = createGameServices();

  window.FantasyGameServicesModule = Object.freeze({
    normalizeGameServiceError,
    createGameServices,
    defaultServices
  });

  window.FantasyGameServices = defaultServices;
})();
