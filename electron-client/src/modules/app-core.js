(function initFantasyAppCoreModule() {
  function createAppCoreModule(deps) {
    const {
      state,
      refs,
      chapters,
      jobs,
      chapterArcs,
      showToast,
      setTextIfChanged,
      ensureCompanionRosterState,
      checkChapterMissionRewards,
      getPendingChapterDecision,
      chapterMissionSummary,
      getChapterArcState,
      updateRegionName,
      updatePositionHUD,
      updateAvatarPlacement,
      refreshAvatarEnvironmentFx,
      refreshWorldMapLayout,
      maybeAutoOpenStoryGuide,
      renderStoryGuideModal,
      renderStoryAndQuests,
      renderBoss,
      renderPanels,
      buildInventory
    } = deps || {};

    if (!state || !refs || !chapters || !jobs) {
      throw new Error("createAppCoreModule requires renderer state and config dependencies.");
    }

    function setView(v) {
      state.currentView = v;
      refs.viewMainMenu?.classList.toggle("view-active", v === "mainMenu");
      refs.viewGameplay?.classList.toggle("view-active", v === "gameplay");
      if (v === "gameplay") {
        requestAnimationFrame(() => {
          requestAnimationFrame(() => {
            refreshWorldMapLayout?.(true);
            maybeAutoOpenStoryGuide?.();
          });
        });
      }
    }

    function openModal(id) {
      document.getElementById(id)?.classList.remove("hidden");
    }

    function closeModal(id) {
      document.getElementById(id)?.classList.add("hidden");
    }

    function chapter() {
      return chapters[Math.max(0, Math.min(chapters.length - 1, state.story.chapterIndex))];
    }

    function professionLabel() {
      return jobs[state.player.profession]?.label || "未知";
    }

    function levelUpIfNeeded() {
      while (state.player.exp >= state.player.level * 100) {
        state.player.exp -= state.player.level * 100;
        state.player.level += 1;
        state.player.maxHp += 12;
        state.player.maxMp += 8;
        state.player.baseAttack += 2;
        state.player.baseDefense += 2;
        state.player.hp = state.player.maxHp;
        state.player.mp = state.player.maxMp;
        state.talents.points = (state.talents.points || 0) + 1;
        showToast?.("升级！Lv." + state.player.level + "（天赋点 +1）");
      }
    }

    function updateHUD() {
      const atk = state.player.baseAttack + state.player.attackBonus;
      const def = state.player.baseDefense + state.player.defenseBonus;
      setTextIfChanged?.(document.getElementById("playerName"), "名称: " + state.player.name);
      setTextIfChanged?.(document.getElementById("playerProfession"), "职业: " + professionLabel());
      setTextIfChanged?.(document.getElementById("playerLevel"), "等级: " + state.player.level);
      setTextIfChanged?.(document.getElementById("playerHP"), "HP: " + state.player.hp + " / " + state.player.maxHp);
      setTextIfChanged?.(document.getElementById("playerMP"), "MP: " + state.player.mp + " / " + state.player.maxMp);
      setTextIfChanged?.(document.getElementById("playerAttack"), "攻击: " + atk);
      setTextIfChanged?.(document.getElementById("playerDefense"), "防御: " + def);
      setTextIfChanged?.(document.getElementById("playerWeapon"), "武器: " + state.player.gear.weapon);
      setTextIfChanged?.(document.getElementById("playerArmor"), "护甲: " + state.player.gear.armor);
      setTextIfChanged?.(document.getElementById("gameChapter"), "章节: " + state.world.chapter);
      setTextIfChanged?.(document.getElementById("gameLocation"), "地点: " + state.world.location);
      setTextIfChanged?.(document.getElementById("gameTime"), "时间: 第 " + state.world.day + " 天");
      setTextIfChanged?.(document.getElementById("gameGold"), "金币: " + state.player.gold);
      setTextIfChanged?.(refs.playerExp, "EXP: " + state.player.exp);
      setTextIfChanged?.(refs.playerPotion, "药水: " + state.player.potions);
      updatePositionHUD?.();
      updateAvatarPlacement?.();
      if (refs.worldMap) {
        const phase = state.world.day % 3;
        refs.worldMap.dataset.time = phase === 0 ? "dawn" : phase === 1 ? "day" : "night";
      }
      refreshAvatarEnvironmentFx?.(false);
    }

    function getMainObjectiveHint(c) {
      if (!c) return "";
      if (c.key === "explore") return "可点击【探索区域】或【触发剧情事件】。";
      if (c.key === "artifact") return "可点击【探索区域】收集碎片或推进主线节点。";
      if (c.key === "boss") return "请点击【开始 Boss 战】后再点击【攻击 Boss】。";
      return "请先继续探索当前章节。";
    }

    function getStoryProgressStatus(c = chapter()) {
      ensureGameplayState();
      const keyLabelMap = { explore: "探索", artifact: "遗物", boss: "首领" };
      const mainKey = keyLabelMap[c.key] || c.key;
      const progress = Number(state.story.progress[c.key] || 0);
      const target = Number(c.target || 0);
      const objectiveRemaining = Math.max(0, target - progress);

      const sideDone = Number(state.storyRuntime.completedSideQuests || 0);
      const sideRequired = Math.max(1, state.story.chapterIndex + 1);
      const sideNeed = Math.max(0, sideRequired - sideDone);

      const arcNodes = chapterArcs[c.id] || [];
      const arcState = getChapterArcState?.(c.id) || { index: 0 };
      const arcRemaining = arcNodes.length ? Math.max(0, arcNodes.length - Number(arcState.index || 0)) : 0;

      const pendingDecision = getPendingChapterDecision?.();
      const mission = chapterMissionSummary?.(c.id) || { total: 0, done: 0, remaining: 0, next: null };
      const blockers = [];

      if (objectiveRemaining > 0) {
        blockers.push("主目标未完成：" + mainKey + " " + progress + "/" + target + "。" + getMainObjectiveHint(c));
      }
      if (arcRemaining > 0) {
        blockers.push("主线节点未完成：还需 " + arcRemaining + " 个，可点击【探索区域】或【触发剧情事件】。");
      }
      if (sideNeed > 0) {
        blockers.push("支线完成数不足：需要 " + sideRequired + " 个，当前 " + sideDone + " 个。可继续探索、刷新委托板，或与 NPC 交流领取委托。");
      }
      if (mission.remaining > 0) {
        const tip = mission.next ? ("优先：" + mission.next.title + "（" + (mission.next.actionLabel || "执行对应操作") + "）") : "请继续完成本章任务";
        blockers.push("章节任务未完成：" + mission.done + "/" + mission.total + "。" + tip);
      }

      const canAdvance = blockers.length === 0;
      let nextAction = canAdvance
        ? "条件已满足，点击【推进章节】即可。"
        : blockers[0];

      if (pendingDecision) {
        nextAction += " 另外，当前有可选抉择：【" + pendingDecision.title + "】。";
      }

      return {
        canAdvance,
        blockers,
        nextAction,
        objectiveRemaining,
        sideNeed,
        arcRemaining,
        mission,
        pendingDecision,
        mainKey,
        progress,
        target
      };
    }

    function currentChapterId() {
      return chapter().id;
    }

    function ensureGameplayState() {
      if (!state.factions) state.factions = { wardens: 0, arcanum: 0, freeguild: 0 };
      if (!Array.isArray(state.sideQuests)) state.sideQuests = [];
      if (!Array.isArray(state.journal)) state.journal = [];
      if (!state.party) state.party = { roster: [], activeId: "", nextId: 1 };
      if (!Array.isArray(state.party.roster)) state.party.roster = [];
      if (!state.talents) state.talents = { offense: 0, guard: 0, support: 0, points: 0 };
      if (!state.storyRuntime) state.storyRuntime = { beats: 0, completedSideQuests: 0, decisions: {}, arc: {}, storyCards: [], chapterReports: [], pinnedStoryCardIds: [], collapsedReplayChapters: [], missionProgress: {}, missionRewards: [], poiVisited: [] };
      if (!state.storyRuntime.decisions) state.storyRuntime.decisions = {};
      if (!state.storyRuntime.arc) state.storyRuntime.arc = {};
      if (!Array.isArray(state.storyRuntime.storyCards)) state.storyRuntime.storyCards = [];
      if (!Array.isArray(state.storyRuntime.chapterReports)) state.storyRuntime.chapterReports = [];
      if (!Array.isArray(state.storyRuntime.pinnedStoryCardIds)) state.storyRuntime.pinnedStoryCardIds = [];
      if (!Array.isArray(state.storyRuntime.collapsedReplayChapters)) state.storyRuntime.collapsedReplayChapters = [];
      if (!state.storyRuntime.missionProgress) state.storyRuntime.missionProgress = {};
      if (!Array.isArray(state.storyRuntime.missionRewards)) state.storyRuntime.missionRewards = [];
      if (!Array.isArray(state.storyRuntime.poiVisited)) state.storyRuntime.poiVisited = [];
      if (!Array.isArray(state.skillLoadout)) state.skillLoadout = ["quick_strike", "guard_break", "aether_burst"];
      if (!state.projects) state.projects = { built: 0, operationsCompleted: [] };
      if (typeof state.projects.built !== "number") state.projects.built = 0;
      if (!Array.isArray(state.projects.operationsCompleted)) state.projects.operationsCompleted = [];
      if (!state.npcs) state.npcs = { activeMap: [], selectedId: "", trust: {}, claimedQuestRewards: [], spawnKey: "", chainFlags: {}, chapterRewards: [] };
      if (!Array.isArray(state.npcs.activeMap)) state.npcs.activeMap = [];
      if (!state.npcs.trust) state.npcs.trust = {};
      if (!Array.isArray(state.npcs.claimedQuestRewards)) state.npcs.claimedQuestRewards = [];
      if (!state.npcs.chainFlags) state.npcs.chainFlags = {};
      if (!Array.isArray(state.npcs.chapterRewards)) state.npcs.chapterRewards = [];
      if (!Array.isArray(state.npcs.relationshipEvents)) state.npcs.relationshipEvents = [];
      ensureCompanionRosterState?.();
    }

    function renderAll() {
      ensureGameplayState();
      checkChapterMissionRewards?.(currentChapterId());
      updateRegionName?.();
      updateHUD();
      renderStoryAndQuests?.();
      renderBoss?.();
      renderPanels?.();
      buildInventory?.();
      if (!document.getElementById("modalStoryGuide")?.classList.contains("hidden")) renderStoryGuideModal?.();
    }

    return Object.freeze({
      setView,
      openModal,
      closeModal,
      chapter,
      professionLabel,
      levelUpIfNeeded,
      updateHUD,
      getMainObjectiveHint,
      getStoryProgressStatus,
      currentChapterId,
      ensureGameplayState,
      renderAll
    });
  }

  window.FantasyAppCoreModule = Object.freeze({
    createAppCoreModule
  });
})();
