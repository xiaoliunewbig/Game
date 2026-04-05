(function initFantasyBackendSyncModule() {
  function createBackendSyncModule(deps) {
    const {
      state,
      refs,
      appendLog,
      showToast,
      services = window.FantasyGameServices,
      storage = window.localStorage
    } = deps || {};

    if (!services) {
      throw new Error("FantasyGameServices failed to load.");
    }

    function renderRules() {
      if (!refs.ruleList) return;
      refs.ruleList.innerHTML = '';
      if (!state.rules.length) {
        refs.ruleList.textContent = '\u6682\u65e0\u53ef\u7528\u89c4\u5219';
        return;
      }
      state.rules.slice(0, 8).forEach((rule) => {
        const div = document.createElement('div');
        div.className = 'rule-item';
        div.textContent = '[' + rule.category + '] ' + rule.rule_id;
        refs.ruleList.appendChild(div);
      });
    }

    function renderKV(container, mapObj) {
      if (!container) return;
      container.innerHTML = '';
      const entries = Object.entries(mapObj || {});
      if (!entries.length) {
        container.textContent = '\u6682\u65e0\u53ef\u5c55\u793a\u6761\u76ee';
        return;
      }
      entries.slice(0, 12).forEach(([key, value]) => {
        const div = document.createElement('div');
        div.className = 'rule-item';
        div.textContent = key + ': ' + value;
        container.appendChild(div);
      });
    }

    async function syncServerState() {
      try {
        const [globalState, flagState] = await Promise.all([
          services.queryGameState({ query_type: 'global_vars', entity_id: 0 }),
          services.queryGameState({ query_type: 'flags', entity_id: 0 })
        ]);
        renderKV(refs.serverVars, globalState.world_state?.global_variables || {});
        renderKV(refs.serverFlags, flagState.world_state?.world_flags || {});
        appendLog('\u5df2\u540c\u6b65\u540e\u7aef\u72b6\u6001');
      } catch (error) {
        appendLog('\u540c\u6b65\u540e\u7aef\u72b6\u6001\u5931\u8d25: ' + (error.details || error.message || error));
      }
    }

    async function reloadRules() {
      try {
        const response = await services.getGameRules({ category: 'all', active_only: true });
        state.rules = Array.isArray(response.rules) ? response.rules : [];
        renderRules();
        showToast('\u5df2\u52a0\u8f7d\u89c4\u5219: ' + state.rules.length);
      } catch (error) {
        showToast('\u89c4\u5219\u52a0\u8f7d\u5931\u8d25: ' + (error.details || error.message || error));
      }
    }

    function createEventRuleRow(id = '', rule = '') {
      if (!refs.eventRuleRows) return;
      const row = document.createElement('div');
      row.className = 'mapping-row';
      row.innerHTML = "<input class='tiny-input map-id' value='" + id + "' placeholder='event id'><input class='tiny-input map-rule' value='" + rule + "' placeholder='rule id'><button class='side-btn compact map-del'>\u5220\u9664</button>";
      row.querySelector('.map-del')?.addEventListener('click', () => {
        row.remove();
        updateRulePreview();
      });
      row.querySelectorAll('input').forEach((input) => input.addEventListener('input', updateRulePreview));
      refs.eventRuleRows.appendChild(row);
    }

    function collectRuleMap() {
      const out = {};
      refs.eventRuleRows?.querySelectorAll('.mapping-row').forEach((row) => {
        const id = row.querySelector('.map-id')?.value?.trim();
        const rule = row.querySelector('.map-rule')?.value?.trim();
        if (id && rule) out[id] = rule;
      });
      return out;
    }

    function updateRulePreview() {
      if (refs.eventRulePreview) {
        refs.eventRulePreview.textContent = JSON.stringify({ event_rule_map: collectRuleMap() }, null, 2);
      }
    }

    function applyRuleMap() {
      storage.setItem('rule_map_local_v3', JSON.stringify(collectRuleMap()));
      showToast('\u6620\u5c04\u5df2\u4fdd\u5b58');
    }

    function exportEventRules() {
      const blob = new Blob([JSON.stringify({ event_rule_map: collectRuleMap() }, null, 2)], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const anchor = document.createElement('a');
      anchor.href = url;
      anchor.download = 'event_rule_map_' + Date.now() + '.json';
      anchor.click();
      URL.revokeObjectURL(url);
    }

    function importEventRulesFile(event) {
      const file = event.target.files?.[0];
      if (!file) return;
      const reader = new FileReader();
      reader.onload = () => {
        try {
          const parsed = JSON.parse(String(reader.result || '{}'));
          refs.eventRuleRows.innerHTML = '';
          Object.entries(parsed.event_rule_map || {}).forEach(([key, value]) => createEventRuleRow(key, String(value)));
          updateRulePreview();
        } catch {
          showToast('\u5bfc\u5165\u89c4\u5219\u6587\u4ef6\u5931\u8d25\uff0c\u8bf7\u68c0\u67e5 JSON \u683c\u5f0f');
        }
      };
      reader.readAsText(file);
      event.target.value = '';
    }

    async function bootstrapFromBackend() {
      if (refs.serviceState) {
        refs.serviceState.textContent = '\u6b63\u5728\u68c0\u67e5\u540e\u7aef\u670d\u52a1...';
      }
      try {
        const payload = await services.bootstrapGame();
        const algorithmOk = Boolean(payload?.health?.algorithm?.ok);
        const strategyOk = Boolean(payload?.health?.strategy?.ok);
        if (refs.serviceState) {
          refs.serviceState.textContent = '\u7b97\u6cd5\u670d\u52a1: ' + (algorithmOk ? '\u5df2\u8fde\u63a5' : '\u672a\u8fde\u63a5') + '\n' +
            '\u7b56\u7565\u670d\u52a1: ' + (strategyOk ? '\u5df2\u8fde\u63a5' : '\u672a\u8fde\u63a5');
          refs.serviceState.className = 'service-state ' + (algorithmOk && strategyOk ? 'ok' : 'warn');
        }
        state.rules = Array.isArray(payload?.rules?.rules) ? payload.rules.rules : [];
        renderRules();
        if (refs.debugWorldState && payload?.state?.state_json) {
          refs.debugWorldState.value = payload.state.state_json;
        }
      } catch (error) {
        if (refs.serviceState) {
          refs.serviceState.textContent = '\u540e\u7aef\u8fde\u63a5\u5931\u8d25\n' + (error.message || error);
          refs.serviceState.className = 'service-state error';
        }
      }
    }

    return Object.freeze({
      renderRules,
      syncServerState,
      reloadRules,
      createEventRuleRow,
      updateRulePreview,
      applyRuleMap,
      exportEventRules,
      importEventRulesFile,
      bootstrapFromBackend
    });
  }

  window.FantasyBackendSyncModule = Object.freeze({
    createBackendSyncModule
  });
})();
