(function initFantasyLocalStateModule() {
  const DEFAULT_LOCAL_STATE_KEYS = Object.freeze({
    settings: 'fantasy_settings_v3',
    save: 'fantasy_local_save_v3',
    storyGuideAuto: 'fantasy_story_guide_auto_v1'
  });

  function createLocalStateModule(deps) {
    const {
      state,
      refs,
      showToast,
      renderAll,
      setView,
      closeModal,
      storage = window.localStorage,
      keys = DEFAULT_LOCAL_STATE_KEYS
    } = deps || {};

    let storyGuideAutoShown = false;

    const readValue = (key) => {
      try {
        return storage.getItem(key);
      } catch {
        return null;
      }
    };

    const writeValue = (key, value) => {
      try {
        storage.setItem(key, value);
      } catch {}
    };

    function buildSaveSlots() {
      if (!refs.saveList) return;
      refs.saveList.innerHTML = '';
      for (let i = 0; i < 6; i += 1) {
        const item = document.createElement('div');
        item.className = 'save-item';
        item.innerHTML = "<span>\u5b58\u6863 " + (i + 1) + "</span><button data-slot='" + i + "'>\u8bfb\u53d6</button>";
        refs.saveList.appendChild(item);
      }

      if (refs.saveList.dataset.boundSaveSlots === '1') return;
      refs.saveList.dataset.boundSaveSlots = '1';
      refs.saveList.addEventListener('click', (event) => {
        const target = event.target;
        if (!(target instanceof HTMLButtonElement)) return;
        const raw = readValue(keys.save + '_' + target.dataset.slot);
        if (!raw) {
          showToast('\u8be5\u5b58\u6863\u4f4d\u4e3a\u7a7a');
          return;
        }

        const snapshot = JSON.parse(raw);
        Object.assign(state, snapshot);
        renderAll();
        setView('gameplay');
        closeModal('modalLoadGame');
        showToast('\u5df2\u8bfb\u53d6\u5b58\u6863\u5e76\u8fdb\u5165\u6e38\u620f');
      });
    }

    function saveToSlot(slot = 0) {
      writeValue(keys.save + '_' + slot, JSON.stringify(state));
      showToast('\u5b58\u6863\u6210\u529f');
    }

    function loadSettings() {
      try {
        Object.assign(state.settings, JSON.parse(readValue(keys.settings) || '{}'));
      } catch {}

      storyGuideAutoShown = readValue(keys.storyGuideAuto) === '1';
      if (refs.volumeRange) refs.volumeRange.value = String(state.settings.volume);
      if (refs.fpsLimit) refs.fpsLimit.value = state.settings.fpsLimit;
    }

    function persistSettings() {
      state.settings.volume = Number(refs.volumeRange?.value || 70);
      state.settings.fpsLimit = refs.fpsLimit?.value || '60';
      writeValue(keys.settings, JSON.stringify(state.settings));
    }

    function loadCurrentStateJson() {
      if (!refs.debugWorldState) return;
      refs.debugWorldState.value = JSON.stringify({
        player: state.player,
        world: state.world,
        story: state.story
      }, null, 2);
    }

    function applyDebugWorldState() {
      try {
        const value = JSON.parse(refs.debugWorldState?.value || '{}');
        if (value.player) Object.assign(state.player, value.player);
        if (value.world) Object.assign(state.world, value.world);
        if (value.story) Object.assign(state.story, value.story);
        renderAll();
        showToast('\u72b6\u6001 JSON \u5df2\u5e94\u7528');
      } catch {
        showToast('\u72b6\u6001 JSON \u683c\u5f0f\u9519\u8bef');
      }
    }

    function isStoryGuideAutoShown() {
      return storyGuideAutoShown;
    }

    function markStoryGuideAutoShown(value = true) {
      storyGuideAutoShown = Boolean(value);
      writeValue(keys.storyGuideAuto, storyGuideAutoShown ? '1' : '0');
    }

    return Object.freeze({
      buildSaveSlots,
      saveToSlot,
      loadSettings,
      persistSettings,
      loadCurrentStateJson,
      applyDebugWorldState,
      isStoryGuideAutoShown,
      markStoryGuideAutoShown,
      keys
    });
  }

  window.FantasyLocalStateModule = Object.freeze({
    DEFAULT_LOCAL_STATE_KEYS,
    createLocalStateModule
  });
})();
