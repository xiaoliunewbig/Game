(function initFantasyGameplayRuntimeModule() {
  function createGameplayRuntimeModule(deps) {
    const {
      state,
      refs,
      showToast,
      appendLog,
      renderAll,
      levelUpIfNeeded
    } = deps || {};

    if (!state || !refs) {
      throw new Error("createGameplayRuntimeModule requires state and refs.");
    }

    function buildShop() {
      state.content.shop = [
        {
          id: "potion",
          name: "\u57fa\u7840\u7597\u4f24\u836f\u5242",
          price: 30,
          apply: () => {
            state.player.potions += 1;
          }
        },
        {
          id: "blade",
          name: "\u7cbe\u94a2\u957f\u5251",
          price: 120,
          apply: () => {
            state.player.attackBonus += 2;
            state.player.gear.weapon = "\u7cbe\u94a2\u957f\u5251";
          }
        },
        {
          id: "armor",
          name: "\u94a2\u7eb9\u80f8\u7532",
          price: 130,
          apply: () => {
            state.player.defenseBonus += 2;
            state.player.gear.armor = "\u94a2\u7eb9\u80f8\u7532";
          }
        }
      ];
      if (refs.shopItemSelect) {
        refs.shopItemSelect.innerHTML = state.content.shop
          .map((item) => "<option value='" + item.id + "'>" + item.name + " - " + item.price + "g</option>")
          .join("");
      }
      if (refs.shopSummary) {
        refs.shopSummary.textContent = "\u5546\u5e97\u5df2\u5237\u65b0";
      }
    }

    function addJournal(text) {
      const line = "[Day " + state.world.day + "] " + text;
      if (!Array.isArray(state.journal)) state.journal = [];
      state.journal.unshift(line);
      state.journal = state.journal.slice(0, 24);
      appendLog?.(text);
    }

    function applyReputation(key, delta) {
      if (!state.factions) state.factions = {};
      if (typeof state.factions[key] !== "number") state.factions[key] = 0;
      state.factions[key] += Number(delta || 0);
    }

    function gain(exp, gold) {
      state.player.exp += Number(exp || 0);
      state.player.gold += Number(gold || 0);
      levelUpIfNeeded?.();
    }

    function restAtCamp() {
      state.world.day += 1;
      state.player.hp = Math.min(state.player.maxHp, state.player.hp + Math.floor(state.player.maxHp * 0.35));
      state.player.mp = Math.min(state.player.maxMp, state.player.mp + Math.floor(state.player.maxMp * 0.45));
      showToast?.("\u4f11\u606f\u5b8c\u6210");
      appendLog?.("\u5728\u8425\u5730\u4f11\u606f\u4e00\u665a\uff0c\u72b6\u6001\u56de\u590d\u3002");
      renderAll?.();
    }

    function usePotion() {
      if (state.player.potions <= 0) {
        return showToast?.("\u836f\u6c34\u4e0d\u8db3");
      }
      state.player.potions -= 1;
      state.player.hp = Math.min(state.player.maxHp, state.player.hp + 45);
      showToast?.("\u836f\u6c34\u5df2\u4f7f\u7528");
      renderAll?.();
    }

    function forgeRelic() {
      if (state.content.artifactShards < 3) {
        return showToast?.("\u788e\u7247\u4e0d\u8db3\uff0c\u81f3\u5c11\u9700\u8981 3 \u4e2a");
      }
      state.content.artifactShards -= 3;
      state.content.forgedRelics += 1;
      state.player.attackBonus += 1;
      state.player.defenseBonus += 1;
      showToast?.("\u9057\u7269\u953b\u9020\u6210\u529f");
      renderAll?.();
    }

    function buyShopItem() {
      const id = refs.shopItemSelect?.value;
      const item = (state.content.shop || []).find((entry) => entry.id === id);
      if (!item) return;
      if (state.player.gold < item.price) {
        return showToast?.("\u91d1\u5e01\u4e0d\u8db3");
      }
      state.player.gold -= item.price;
      item.apply();
      showToast?.("\u8d2d\u4e70\u6210\u529f: " + item.name);
      renderAll?.();
    }

    function buildInventory() {
      if (!refs.inventoryGrid) return;
      refs.inventoryGrid.innerHTML = "";
      const summaryItems = [
        state.player.potions > 0 ? "\u836f\u6c34 x" + state.player.potions : null,
        state.content.artifactShards > 0 ? "\u9057\u7269\u788e\u7247 x" + state.content.artifactShards : null,
        state.content.forgedRelics > 0 ? "\u953b\u9020\u9057\u7269 x" + state.content.forgedRelics : null,
        state.player.gear.weapon ? "\u6b66\u5668: " + state.player.gear.weapon : null,
        state.player.gear.armor ? "\u62a4\u7532: " + state.player.gear.armor : null
      ].filter(Boolean);

      for (let i = 0; i < 24; i += 1) {
        const slot = document.createElement("div");
        slot.className = "inventory-item";
        slot.textContent = summaryItems[i] || "\u7a7a";
        refs.inventoryGrid.appendChild(slot);
      }
    }

    return Object.freeze({
      addJournal,
      applyReputation,
      buildInventory,
      buildShop,
      buyShopItem,
      forgeRelic,
      gain,
      restAtCamp,
      usePotion
    });
  }

  window.FantasyGameplayRuntimeModule = Object.freeze({
    createGameplayRuntimeModule
  });
})();