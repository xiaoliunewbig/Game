(function initFantasyAppShellModule() {
  function createAppShellModule(deps) {
    const {
      state,
      refs,
      jobs,
      localState,
      backendSync,
      safeOn,
      showToast,
      setView,
      openModal,
      closeModal,
      ensureGameplayState,
      currentChapterId,
      renderAll,
      buildWorldTiles,
      buildInventory,
      buildShop,
      refreshSideQuests,
      refreshWorldMapLayout,
      applyChineseStoryContent,
      applyChineseUiText,
      renderStoryReplay,
      renderChapterSummary,
      storyTextData,
      actions
    } = deps || {};

    if (!state || !refs || !jobs || !localState || !backendSync || !actions) {
      throw new Error("createAppShellModule requires core renderer dependencies.");
    }

    let resizeTimer = null;

    function bindEvents() {
      safeOn(refs.btnNewGame, "click", () => openModal("modalNewGame"));
      safeOn(refs.btnLoadGame, "click", () => openModal("modalLoadGame"));
      safeOn(refs.btnSettings, "click", () => openModal("modalSettings"));
      safeOn(refs.btnExit, "click", () => window.close());
      safeOn(refs.btnInventory, "click", () => refs.inventoryDrawer?.classList.add("open"));
      safeOn(refs.closeInventory, "click", () => refs.inventoryDrawer?.classList.remove("open"));
      safeOn(refs.btnQuest, "click", () => {
        refreshSideQuests();
        showToast("委托面板已刷新");
      });
      safeOn(refs.btnSkillTree, "click", actions.showTalentSummary);
      safeOn(refs.btnSave, "click", () => localState.saveToSlot(0));
      safeOn(refs.btnBackMenu, "click", () => setView("mainMenu"));
      safeOn(refs.createCharacter, "click", () => {
        const name = refs.inputPlayerName?.value?.trim();
        if (!name) return showToast("请输入角色名");
        state.player.name = name;
        state.player.profession = state.selectedProfession;
        const job = jobs[state.selectedProfession] || jobs.warrior;
        state.player.maxHp = job.hp;
        state.player.maxMp = job.mp;
        state.player.baseAttack = job.atk;
        state.player.baseDefense = job.def;
        state.player.hp = job.hp;
        state.player.mp = job.mp;
        closeModal("modalNewGame");
        setView("gameplay");
        renderAll();
        showToast("欢迎，" + name);
      });
      safeOn(refs.saveSettings, "click", () => {
        localState.persistSettings();
        closeModal("modalSettings");
        showToast("设置已保存");
      });

      safeOn(refs.btnExplore, "click", actions.exploreArea);
      safeOn(refs.btnRest, "click", actions.restAtCamp);
      safeOn(refs.btnUsePotion, "click", actions.usePotion);
      safeOn(refs.btnAdvanceChapter, "click", actions.advanceChapter);
      safeOn(refs.btnCommitChoice, "click", actions.commitChoice);
      safeOn(refs.btnStartBoss, "click", actions.startBoss);
      safeOn(refs.btnStrikeBoss, "click", actions.strikeBoss);
      safeOn(refs.btnClearBossTimeline, "click", actions.clearBossTimeline);
      safeOn(refs.btnRunDamage, "click", actions.runDamageSimulation);
      safeOn(refs.btnRunAI, "click", actions.runAIDecision);
      safeOn(refs.btnReloadRules, "click", backendSync.reloadRules);
      safeOn(refs.btnSyncState, "click", backendSync.syncServerState);
      safeOn(refs.btnLoadStateJson, "click", localState.loadCurrentStateJson);
      safeOn(refs.btnApplyStateJson, "click", localState.applyDebugWorldState);
      safeOn(refs.btnRefreshShop, "click", () => {
        buildShop();
        showToast("商店已刷新");
      });
      safeOn(refs.btnBuyShopItem, "click", actions.buyShopItem);
      safeOn(refs.btnForgeRelic, "click", actions.forgeRelic);
      safeOn(refs.btnBuildProject, "click", () => {
        const result = actions.completeSupportOperation(currentChapterId());
        if (!result?.ok) {
          showToast(result?.reason || "当前条件尚未满足，无法执行协作行动。");
        }
      });
      safeOn(refs.btnCampfireStory, "click", () => {
        ensureGameplayState();
        const chapterId = currentChapterId();
        const pool = actions.getCampfirePool(chapterId);
        const idx = (state.world.day + state.storyRuntime.beats + (state.projects?.built || 0)) % pool.length;
        const scene = pool[idx];
        state.player.mp = Math.min(state.player.maxMp, state.player.mp + 10);
        actions.gain(10, 8);
        state.storyRuntime.beats += 1;
        const mate = actions.activeCompanion();
        if (mate) {
          actions.grantCompanionGrowth("营火对话", 12, 1, { companionId: mate.id, log: false, toast: false });
          actions.addJournal(scene + " 你与 " + mate.name + " 在营火旁交换了情报。");
        } else {
          actions.addJournal(scene);
        }
        showToast("营火剧情已推进");
        renderAll();
      });
      safeOn(refs.btnRecruitCompanion, "click", actions.recruitCompanion);
      safeOn(refs.companionSelect, "change", () => {
        state.party.activeId = refs.companionSelect?.value || "";
        actions.renderPanels();
      });
      safeOn(refs.btnTalentInfo, "click", actions.showTalentSummary);
      safeOn(refs.btnTalentOffense, "click", () => actions.applyTalent("offense"));
      safeOn(refs.btnTalentGuard, "click", () => actions.applyTalent("guard"));
      safeOn(refs.btnTalentSupport, "click", () => actions.applyTalent("support"));
      safeOn(refs.btnTalentAuto, "click", actions.autoAllocateTalents);
      safeOn(refs.btnResetBuild, "click", actions.resetBuild);
      safeOn(refs.btnRefreshSideQuests, "click", refreshSideQuests);
      safeOn(refs.btnTriggerStory, "click", actions.triggerStoryEvent);
      safeOn(refs.btnTriggerCombat, "click", actions.triggerCombatEvent);
      safeOn(refs.btnStoryDecision, "click", actions.openStoryDecisionModal);
      safeOn(refs.btnStoryGuide, "click", actions.openStoryGuideModal);
      safeOn(refs.btnOpenNpcDialog, "click", actions.openNearestNpcDialog);
      safeOn(refs.btnNpcTalk, "click", actions.npcActionTalk);
      safeOn(refs.btnNpcQuest, "click", actions.npcActionQuest);
      safeOn(refs.btnNpcTrade, "click", actions.npcActionTrade);
      safeOn(refs.btnNpcTrain, "click", actions.npcActionTrain);
      safeOn(refs.btnNpcGift, "click", actions.npcActionGift);
      safeOn(refs.btnStoryGuideAction, "click", actions.runStoryGuideAction);
      safeOn(refs.btnStoryReplay, "click", actions.openStoryReplayModal);
      safeOn(refs.selectStoryReplayType, "change", renderStoryReplay);
      safeOn(refs.inputStoryReplaySearch, "input", renderStoryReplay);
      safeOn(refs.toggleStoryPinnedOnly, "change", renderStoryReplay);
      safeOn(refs.toggleStoryGroupByChapter, "change", renderStoryReplay);
      safeOn(refs.btnExportStoryReplayTxt, "click", () => actions.exportStoryReplay(false));
      safeOn(refs.btnExportStoryReplayJson, "click", () => actions.exportStoryReplay(true));
      safeOn(refs.btnChapterSummary, "click", actions.openChapterSummaryModal);
      safeOn(refs.toggleChapterCompare, "change", renderChapterSummary);
      safeOn(refs.compareChapterA, "change", renderChapterSummary);
      safeOn(refs.compareChapterB, "change", renderChapterSummary);
      safeOn(refs.btnExportChapterSummaryTxt, "click", () => actions.exportChapterSummary(false));
      safeOn(refs.btnExportChapterSummaryJson, "click", () => actions.exportChapterSummary(true));
      safeOn(refs.btnViewEndingReview, "click", actions.openEndingReviewModal);
      safeOn(refs.btnExportEndingTxt, "click", () => actions.exportEndingReview(false));
      safeOn(refs.btnExportEndingJson, "click", () => actions.exportEndingReview(true));
      safeOn(refs.btnCastSkill, "click", actions.castSelectedSkill);
      safeOn(refs.btnCompanionSkill, "click", actions.castCompanionSkill);

      safeOn(refs.btnAddEventRule, "click", () => {
        backendSync.createEventRuleRow("", "");
        backendSync.updateRulePreview();
      });
      safeOn(refs.btnApplyEventRules, "click", backendSync.applyRuleMap);
      safeOn(refs.btnExportEventRules, "click", backendSync.exportEventRules);
      safeOn(refs.btnImportEventRules, "click", () => refs.inputEventRulesFile?.click());
      safeOn(refs.inputEventRulesFile, "change", backendSync.importEventRulesFile);

      document.querySelectorAll("[data-close]").forEach((button) => {
        safeOn(button, "click", () => closeModal(button.dataset.close));
      });
      document.querySelectorAll(".profession").forEach((button) => {
        safeOn(button, "click", () => {
          document.querySelectorAll(".profession").forEach((node) => node.classList.remove("selected"));
          button.classList.add("selected");
          state.selectedProfession = button.dataset.profession || "warrior";
        });
      });

      safeOn(window, "keydown", (event) => {
        const key = event.key.toLowerCase();
        if (key === "w" || key === "arrowup") actions.movePlayer(0, -state.world.step);
        if (key === "s" || key === "arrowdown") actions.movePlayer(0, state.world.step);
        if (key === "a" || key === "arrowleft") actions.movePlayer(-state.world.step, 0);
        if (key === "d" || key === "arrowright") actions.movePlayer(state.world.step, 0);
        if (key === "e") actions.interactNearestRegionPoi();
      });

      safeOn(refs.worldMap, "dblclick", (event) => {
        event.preventDefault();
        actions.teleportPlayerTo(event.clientX, event.clientY);
      });

      safeOn(refs.worldMap, "contextmenu", (event) => {
        event.preventDefault();
        actions.teleportPlayerTo(event.clientX, event.clientY);
      });

      safeOn(window, "resize", () => {
        clearTimeout(resizeTimer);
        resizeTimer = setTimeout(() => refreshWorldMapLayout(false), 120);
      });
    }

    function bootstrap() {
      localState.loadSettings();
      applyChineseStoryContent(storyTextData);
      buildWorldTiles();
      buildInventory();
      localState.buildSaveSlots();
      buildShop();
      bindEvents();
      applyChineseUiText({ refs });
      ensureGameplayState();
      if (!state.sideQuests.length) refreshSideQuests();
      backendSync.createEventRuleRow("1001", "story_chapter_1");
      backendSync.createEventRuleRow("2001", "combat_start");
      backendSync.updateRulePreview();
      renderAll();
      renderStoryReplay();
      renderChapterSummary();
      refreshWorldMapLayout(false);
      backendSync.bootstrapFromBackend();
    }

    return Object.freeze({
      bindEvents,
      bootstrap
    });
  }

  window.FantasyAppShellModule = Object.freeze({
    createAppShellModule
  });
})();
