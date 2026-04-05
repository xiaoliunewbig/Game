(function initFantasyProgressionRuntimeModule() {
  const {
    COMPANION_ARCHETYPES,
    CHAPTER_MISSIONS,
    CHAPTER_SUPPORT_OPERATIONS
  } = window.FantasyGameData || {};

  function createProgressionRuntimeModule(deps) {
    const {
      state,
      refs,
      showToast,
      renderAll,
      renderPanels,
      ensureGameplayState,
      currentChapterId,
      chapter,
      addJournal,
      applyReputation,
      gain,
      applyDecisionReward,
      pushStoryCard,
      formatRewardInline,
      regionPoiSummary,
      npcChapterChainSummary,
      npcChapterRelationshipSummary,
      openStoryGuideModal,
      openNearestNpcDialog
    } = deps || {};

    if (!state || !refs) {
      throw new Error("createProgressionRuntimeModule requires state and refs.");
    }

    function companionExpForLevel(level) {
      return 36 + Math.max(0, Number(level || 1) - 1) * 24;
    }

    function ensureCompanionState(mate) {
      if (!mate) return null;
      const base = COMPANION_ARCHETYPES.find((entry) => entry.key === mate.key) || COMPANION_ARCHETYPES[0];
      if (!mate.key) mate.key = base.key;
      if (!mate.name) mate.name = base.name;
      if (!mate.role) mate.role = base.role;
      if (typeof mate.atk !== "number") mate.atk = Number(base.atk || 0);
      if (typeof mate.def !== "number") mate.def = Number(base.def || 0);
      if (typeof mate.support !== "number") mate.support = Number(base.support || 0);
      if (!mate.specialty) mate.specialty = base.specialty || "未知专长";
      if (!mate.field) mate.field = base.field || "未知领域";
      if (!mate.tactic) mate.tactic = base.tactic || "尚未形成战术风格";
      if (!mate.skillName) mate.skillName = base.skillName || "未知技能";
      if (!mate.passive) mate.passive = base.passive || "尚未觉醒被动";
      if (typeof mate.level !== "number" || mate.level < 1) mate.level = 1;
      if (typeof mate.exp !== "number" || mate.exp < 0) mate.exp = 0;
      if (typeof mate.nextExp !== "number" || mate.nextExp < 1) mate.nextExp = companionExpForLevel(mate.level);
      if (typeof mate.bond !== "number" || mate.bond < 1) mate.bond = 1;
      mate.bond = Math.max(1, Math.min(10, mate.bond));
      return mate;
    }

    function ensureCompanionRosterState() {
      if (!state.party) return;
      let maxNumericId = 0;
      state.party.roster = (state.party.roster || []).map((mate, idx) => {
        if (!mate.id) mate.id = "cp_" + (idx + 1);
        const match = /^cp_(\d+)$/.exec(String(mate.id || ""));
        if (match) maxNumericId = Math.max(maxNumericId, Number(match[1]));
        return ensureCompanionState(mate);
      });
      state.party.nextId = Math.max(Number(state.party.nextId || 1), maxNumericId + 1, state.party.roster.length + 1);
      if (!state.party.activeId || !state.party.roster.some((mate) => mate.id === state.party.activeId)) {
        state.party.activeId = state.party.roster[0]?.id || "";
      }
    }

    function companionPower(mate) {
      if (!mate) return 0;
      return mate.level * 6 + mate.bond * 3 + mate.atk * 5 + mate.def * 4 + mate.support * 4;
    }

    function activeCompanion() {
      ensureGameplayState?.();
      ensureCompanionRosterState();
      return state.party.roster.find((entry) => entry.id === state.party.activeId) || null;
    }

    function activeCompanionSummary() {
      const mate = activeCompanion();
      if (!mate) return "还没有招募同伴，可以先去委托板或 NPC 面板推进招募。";
      return mate.name + " | Lv." + mate.level + " | " + mate.role + " | 羁绊 " + mate.bond + "\n"
        + "专长: " + mate.specialty + " | 活跃区域: " + mate.field + "\n"
        + "技能: " + mate.skillName + "\n"
        + "战术: " + mate.tactic + "\n"
        + "被动: " + mate.passive;
    }

    function applyCompanionLevelBonus(mate) {
      if (!mate) return;
      if (mate.key === "guardian") {
        mate.def += 1;
        if (mate.level % 2 === 0) mate.support += 1;
        return;
      }
      if (mate.key === "ranger") {
        mate.atk += 1;
        if (mate.level % 3 === 0) mate.def += 1;
        return;
      }
      if (mate.key === "mystic") {
        mate.support += 1;
        if (mate.level % 2 === 0) mate.atk += 1;
        return;
      }
      mate.atk += 1;
      if (mate.level % 2 === 0) mate.def += 1;
    }

    function grantCompanionGrowth(reason, exp = 0, bondDelta = 0, options = {}) {
      ensureGameplayState?.();
      ensureCompanionRosterState();
      const companionId = options.companionId || state.party.activeId || state.party.roster[0]?.id || "";
      const mate = state.party.roster.find((entry) => entry.id === companionId) || null;
      if (!mate) return null;

      const safeExp = Math.max(0, Math.floor(Number(exp || 0)));
      const safeBond = Math.max(0, Math.floor(Number(bondDelta || 0)));
      let gainedBond = 0;
      let gainedLevels = 0;

      if (safeBond > 0) {
        const beforeBond = mate.bond;
        mate.bond = Math.min(10, mate.bond + safeBond);
        gainedBond = mate.bond - beforeBond;
      }

      if (safeExp > 0) {
        mate.exp += safeExp;
        while (mate.exp >= mate.nextExp) {
          mate.exp -= mate.nextExp;
          mate.level += 1;
          applyCompanionLevelBonus(mate);
          mate.nextExp = companionExpForLevel(mate.level);
          gainedLevels += 1;
        }
      }

      const parts = [];
      if (safeExp > 0) parts.push("经验 +" + safeExp);
      if (gainedBond > 0) parts.push("羁绊 +" + gainedBond);
      if (gainedLevels > 0) parts.push("升至 Lv." + mate.level);

      if (parts.length && options.log !== false) {
        addJournal?.("同伴成长: " + mate.name + " - " + reason + " | " + parts.join(" | "));
      }
      if (gainedLevels > 0 && options.toast !== false) {
        showToast?.(mate.name + " 已提升至 Lv." + mate.level);
      }

      return mate;
    }

    function chapterMissionDefs(chapterId = currentChapterId()) {
      return CHAPTER_MISSIONS[chapterId] || [];
    }

    function chapterMissionState(chapterId = currentChapterId()) {
      ensureGameplayState?.();
      if (!state.storyRuntime.missionProgress) state.storyRuntime.missionProgress = {};
      if (!state.storyRuntime.missionProgress[chapterId]) state.storyRuntime.missionProgress[chapterId] = {};
      return state.storyRuntime.missionProgress[chapterId];
    }

    function missionMetricValue(metric, chapterId = currentChapterId()) {
      return Number(chapterMissionState(chapterId)[metric] || 0);
    }

    function missionDone(def, chapterId = currentChapterId()) {
      return missionMetricValue(def.metric, chapterId) >= Number(def.target || 0);
    }

    function chapterMissionSummary(chapterId = currentChapterId()) {
      const defs = chapterMissionDefs(chapterId);
      if (!defs.length) return { total: 0, done: 0, remaining: 0, next: null, defs: [] };
      let done = 0;
      let next = null;
      for (const def of defs) {
        if (missionDone(def, chapterId)) done += 1;
        else if (!next) next = def;
      }
      return { total: defs.length, done, remaining: defs.length - done, next, defs };
    }

    function applyMissionReward(reward) {
      if (!reward) return;
      applyDecisionReward?.(reward);
      const rep = reward.rep || {};
      if (typeof rep.wardens === "number") applyReputation?.("wardens", rep.wardens);
      if (typeof rep.arcanum === "number") applyReputation?.("arcanum", rep.arcanum);
      if (typeof rep.freeguild === "number") applyReputation?.("freeguild", rep.freeguild);
    }

    function checkChapterMissionRewards(chapterId = currentChapterId()) {
      ensureGameplayState?.();
      const defs = chapterMissionDefs(chapterId);
      if (!defs.length) return;
      if (!Array.isArray(state.storyRuntime.missionRewards)) state.storyRuntime.missionRewards = [];
      if (!Array.isArray(state.storyRuntime.poiVisited)) state.storyRuntime.poiVisited = [];
      for (const def of defs) {
        if (!missionDone(def, chapterId)) continue;
        const rewardId = "mission_" + chapterId + "_" + def.id;
        if (state.storyRuntime.missionRewards.includes(rewardId)) continue;
        state.storyRuntime.missionRewards.push(rewardId);
        applyMissionReward(def.reward || {});
        addJournal?.("章节任务完成: " + def.title);
        pushStoryCard?.("event", "章节任务完成: " + def.title, def.title + " 已完成，阶段奖励已经发放。");
        showToast?.("任务完成: " + def.title);
      }
    }

    function addMissionMetric(metric, amount = 1, chapterId = currentChapterId()) {
      const progress = chapterMissionState(chapterId);
      progress[metric] = Math.max(0, Number(progress[metric] || 0) + amount);
      checkChapterMissionRewards(chapterId);
    }

    function chapterSupportOperationDefs(chapterId = currentChapterId()) {
      return CHAPTER_SUPPORT_OPERATIONS[chapterId] || [];
    }

    function supportOperationStateId(chapterId, operationId) {
      return String(chapterId) + "::support::" + String(operationId);
    }

    function hasCompletedSupportOperation(chapterId, operationId) {
      ensureGameplayState?.();
      return state.projects.operationsCompleted.includes(supportOperationStateId(chapterId, operationId));
    }

    function getCurrentSupportOperation(chapterId = currentChapterId()) {
      const defs = chapterSupportOperationDefs(chapterId);
      return defs.find((def) => !hasCompletedSupportOperation(chapterId, def.id)) || null;
    }

    function getSupportOperationStatus(operation, chapterId = currentChapterId()) {
      ensureGameplayState?.();
      const chain = npcChapterChainSummary?.(chapterId) || { completed: 0 };
      const relation = npcChapterRelationshipSummary?.(chapterId) || { unlocked: 0 };
      const mission = chapterMissionSummary(chapterId);
      const poi = regionPoiSummary?.(chapterId) || { done: 0 };
      const sideDone = Number(state.storyRuntime.completedSideQuests || 0);
      const mate = activeCompanion();
      const blockers = [];

      if (!operation) {
        return { ok: false, completed: true, blockers: ["当前章节的协作行动已经全部完成"], chain, relation, mission, poi, sideDone, mate };
      }

      const req = operation.require || {};
      if (req.chainCompleted && chain.completed < req.chainCompleted) blockers.push("NPC 剧情线 " + chain.completed + "/" + req.chainCompleted);
      if (req.relationUnlocked && relation.unlocked < req.relationUnlocked) blockers.push("关系事件 " + relation.unlocked + "/" + req.relationUnlocked);
      if (req.missionDone && mission.done < req.missionDone) blockers.push("章节任务 " + mission.done + "/" + req.missionDone);
      if (req.poiVisited && poi.done < req.poiVisited) blockers.push("区域地标 " + poi.done + "/" + req.poiVisited);
      if (req.sideQuests && sideDone < req.sideQuests) blockers.push("支线委托 " + sideDone + "/" + req.sideQuests);
      if (req.requireCompanion && !mate) blockers.push("需要至少一名同行同伴");
      if (req.minLevel && state.player.level < req.minLevel) blockers.push("角色等级 " + state.player.level + "/" + req.minLevel);

      return {
        ok: blockers.length === 0,
        completed: hasCompletedSupportOperation(chapterId, operation.id),
        blockers,
        chain,
        relation,
        mission,
        poi,
        sideDone,
        mate
      };
    }

    function chapterSupportSummary(chapterId = currentChapterId()) {
      const defs = chapterSupportOperationDefs(chapterId);
      const completed = defs.filter((def) => hasCompletedSupportOperation(chapterId, def.id)).length;
      const current = getCurrentSupportOperation(chapterId);
      return {
        total: defs.length,
        completed,
        remaining: Math.max(0, defs.length - completed),
        current,
        currentStatus: current ? getSupportOperationStatus(current, chapterId) : null
      };
    }

    function supportRequirementRows(operation, status) {
      if (!operation) return [];
      const req = operation.require || {};
      const rows = [];
      if (req.chainCompleted) {
        rows.push({
          key: "chain",
          label: "NPC 剧情线",
          current: Number(status?.chain?.completed || 0),
          target: Number(req.chainCompleted),
          done: Number(status?.chain?.completed || 0) >= Number(req.chainCompleted),
          hint: "前往对应 NPC 推进剧情线"
        });
      }
      if (req.relationUnlocked) {
        rows.push({
          key: "relation",
          label: "关系事件",
          current: Number(status?.relation?.unlocked || 0),
          target: Number(req.relationUnlocked),
          done: Number(status?.relation?.unlocked || 0) >= Number(req.relationUnlocked),
          hint: "提升信任后可解锁更多关系事件"
        });
      }
      if (req.missionDone) {
        rows.push({
          key: "mission",
          label: "章节任务",
          current: Number(status?.mission?.done || 0),
          target: Number(req.missionDone),
          done: Number(status?.mission?.done || 0) >= Number(req.missionDone),
          hint: status?.mission?.next ? ("优先完成任务: " + status.mission.next.title) : "章节任务已经全部完成"
        });
      }
      if (req.poiVisited) {
        rows.push({
          key: "poi",
          label: "区域地标",
          current: Number(status?.poi?.done || 0),
          target: Number(req.poiVisited),
          done: Number(status?.poi?.done || 0) >= Number(req.poiVisited),
          hint: status?.poi?.nearest ? ("建议先探索: " + (status.poi.nearest.def?.name || status.poi.nearest.id)) : "当前区域地标已经全部探索"
        });
      }
      if (req.sideQuests) {
        rows.push({
          key: "side",
          label: "支线委托",
          current: Number(status?.sideDone || 0),
          target: Number(req.sideQuests),
          done: Number(status?.sideDone || 0) >= Number(req.sideQuests),
          hint: "刷新委托板并完成支线任务"
        });
      }
      if (req.requireCompanion) {
        rows.push({
          key: "companion",
          label: "同行同伴",
          current: status?.mate ? 1 : 0,
          target: 1,
          done: Boolean(status?.mate),
          hint: status?.mate ? ("当前同行同伴: " + status.mate.name) : "请先招募并选择一名同行同伴"
        });
      }
      if (req.minLevel) {
        rows.push({
          key: "level",
          label: "角色等级",
          current: Number(state.player.level || 0),
          target: Number(req.minLevel),
          done: Number(state.player.level || 0) >= Number(req.minLevel),
          hint: "继续探索、推进剧情或挑战 Boss 提升等级"
        });
      }
      return rows;
    }

    function supportPrimaryBlockerText(operation, status) {
      const rows = supportRequirementRows(operation, status);
      const next = rows.find((row) => !row.done);
      if (next) {
        return next.label + " " + next.current + "/" + next.target + (next.hint ? (" | " + next.hint) : "");
      }
      return status?.blockers?.[0] || "仍需完成前置条件";
    }

    function applySupportOperationReward(operation, chapterId = currentChapterId()) {
      if (!operation) return;
      applyMissionReward(operation.reward || {});
      const missionReward = operation.reward?.mission || {};
      for (const [metric, amount] of Object.entries(missionReward)) {
        addMissionMetric(metric, Number(amount || 0), chapterId);
      }
    }

    function buildGenericRegionalProject(chapterId = currentChapterId()) {
      const genericBuilt = Math.max(0, Number(state.projects.built || 0) - Number((state.projects.operationsCompleted || []).length));
      const costGold = 80 + genericBuilt * 30;
      const costShards = chapterId === "camp" ? 0 : 1;
      if (state.player.gold < costGold) return { ok: false, reason: "金币不足: " + costGold };
      if (state.content.artifactShards < costShards) return { ok: false, reason: "碎片不足: " + costShards };

      state.player.gold -= costGold;
      state.content.artifactShards -= costShards;
      state.projects.built += 1;
      if (chapterId === "camp") {
        state.player.maxHp += 8;
        state.player.hp = Math.min(state.player.maxHp, state.player.hp + 8);
        applyReputation?.("wardens", 1);
        addJournal?.("你们加固了营地外围的木墙与篝火塔，守望者的巡逻效率明显提升。");
      } else if (chapterId === "forest") {
        state.player.maxMp += 8;
        state.player.mp = Math.min(state.player.maxMp, state.player.mp + 8);
        applyReputation?.("arcanum", 1);
        addJournal?.("你们修复了林地观测站的法阵回路，奥术议会重新记录裂隙波动。");
      } else {
        state.player.attackBonus += 1;
        state.player.defenseBonus += 1;
        applyReputation?.("freeguild", 1);
        addJournal?.("你们在裂隙前线搭起防御工事与补给点，自由行会的商路开始恢复。");
      }
      gain?.(16, 0);
      addMissionMetric("projects_completed", 1, chapterId);
      showToast?.("地区建设完成");
      renderAll?.();
      return { ok: true, generic: true };
    }

    function completeSupportOperation(chapterId = currentChapterId()) {
      ensureGameplayState?.();
      const operation = getCurrentSupportOperation(chapterId);
      if (!operation) {
        return buildGenericRegionalProject(chapterId);
      }

      const status = getSupportOperationStatus(operation, chapterId);
      if (!status.ok) {
        return { ok: false, reason: supportPrimaryBlockerText(operation, status), operation, status };
      }
      if (state.player.gold < Number(operation.costGold || 0)) {
        return { ok: false, reason: "金币不足: " + Number(operation.costGold || 0), operation, status };
      }
      if (state.content.artifactShards < Number(operation.costShards || 0)) {
        return { ok: false, reason: "碎片不足: " + Number(operation.costShards || 0), operation, status };
      }

      state.player.gold -= Number(operation.costGold || 0);
      state.content.artifactShards -= Number(operation.costShards || 0);
      state.projects.built += 1;
      state.projects.operationsCompleted.push(supportOperationStateId(chapterId, operation.id));
      applySupportOperationReward(operation, chapterId);
      addMissionMetric("projects_completed", 1, chapterId);
      state.storyRuntime.beats += 1;

      const mate = activeCompanion();
      if (mate) {
        grantCompanionGrowth(operation.title, 20, 1, { companionId: mate.id, log: false, toast: false });
      }

      const rewardText = formatRewardInline?.(operation.reward || {});
      addJournal?.(operation.journal || (chapter().name + " 的协作行动已完成: " + operation.title));
      pushStoryCard?.("chapter", "章节协作完成: " + operation.title, operation.desc + (rewardText ? ("\n奖励: " + rewardText) : ""));
      showToast?.("协作完成: " + operation.title);
      renderAll?.();
      return { ok: true, operation, status };
    }

    function ensureSupportTrackerMount() {
      const panel = refs.projectSummary?.closest(".panel-block");
      if (!panel) return null;
      let tracker = document.getElementById("supportTracker");
      if (!tracker) {
        tracker = document.createElement("div");
        tracker.id = "supportTracker";
        tracker.className = "support-tracker";
        panel.insertBefore(tracker, refs.sideQuestList || refs.journalLog || null);
      }
      return tracker;
    }

    function renderSupportTracker() {
      const tracker = ensureSupportTrackerMount();
      if (!tracker) return;
      const chapterId = currentChapterId();
      const defs = chapterSupportOperationDefs(chapterId);
      const summary = chapterSupportSummary(chapterId);
      tracker.innerHTML = "";

      const overview = document.createElement("div");
      overview.className = "operation-overview";
      overview.textContent = defs.length
        ? ("协作进度 " + summary.completed + "/" + summary.total + (summary.current ? (" | 当前 " + summary.current.title) : " | 已全部完成"))
        : "当前章节暂无协作行动";
      tracker.appendChild(overview);

      if (!defs.length) return;

      defs.forEach((operation, index) => {
        const status = getSupportOperationStatus(operation, chapterId);
        const rows = supportRequirementRows(operation, status);
        const isCurrent = summary.current?.id === operation.id;
        const rewardText = formatRewardInline?.(operation.reward || {});

        const card = document.createElement("div");
        card.className = "operation-card"
          + (status.completed ? " done" : "")
          + (isCurrent ? " current" : "")
          + (!status.completed && status.ok ? " ready" : "");

        const header = document.createElement("div");
        header.className = "operation-header";

        const titleWrap = document.createElement("div");
        const title = document.createElement("div");
        title.className = "operation-title";
        title.textContent = operation.title;
        const meta = document.createElement("div");
        meta.className = "operation-meta";
        meta.textContent = "阶段 " + (index + 1) + " | 花费 金币 " + Number(operation.costGold || 0) + (Number(operation.costShards || 0) ? (" | 碎片 " + Number(operation.costShards || 0)) : "");
        titleWrap.appendChild(title);
        titleWrap.appendChild(meta);

        const badge = document.createElement("span");
        badge.className = "operation-badge";
        badge.textContent = status.completed ? "已完成" : (isCurrent ? (status.ok ? "可执行" : "待准备") : "后续");
        header.appendChild(titleWrap);
        header.appendChild(badge);
        card.appendChild(header);

        const desc = document.createElement("div");
        desc.className = "operation-desc";
        desc.textContent = operation.desc || "暂无说明";
        card.appendChild(desc);

        const steps = document.createElement("div");
        steps.className = "operation-step-list";
        rows.forEach((row) => {
          const item = document.createElement("div");
          item.className = "operation-step" + (row.done ? " done" : "");
          item.textContent = (row.done ? "已达成 | " : "未完成 | ") + row.label + ": " + row.current + "/" + row.target + (row.hint ? (" | " + row.hint) : "");
          steps.appendChild(item);
        });
        if (!rows.length) {
          const item = document.createElement("div");
          item.className = "operation-step done";
          item.textContent = "已满足基础要求";
          steps.appendChild(item);
        }
        card.appendChild(steps);

        const note = document.createElement("div");
        note.className = "operation-note";
        if (status.completed) {
          note.textContent = rewardText ? ("奖励已领取: " + rewardText) : "该行动已完成";
        } else if (status.ok) {
          note.textContent = "条件已满足，可立即执行" + (rewardText ? (" | 奖励 " + rewardText) : "");
        } else {
          note.textContent = "仍需准备: " + supportPrimaryBlockerText(operation, status);
        }
        card.appendChild(note);

        const actions = document.createElement("div");
        actions.className = "operation-action-row";

        const guideBtn = document.createElement("button");
        guideBtn.className = "side-btn compact";
        guideBtn.textContent = "查看指引";
        guideBtn.addEventListener("click", () => openStoryGuideModal?.());
        actions.appendChild(guideBtn);

        const npcBtn = document.createElement("button");
        npcBtn.className = "side-btn compact";
        npcBtn.textContent = "前往 NPC";
        npcBtn.addEventListener("click", () => openNearestNpcDialog?.());
        actions.appendChild(npcBtn);

        if (!status.completed && isCurrent) {
          const execBtn = document.createElement("button");
          execBtn.className = "side-btn compact";
          execBtn.textContent = status.ok ? (operation.actionLabel || "执行协作") : "条件不足";
          execBtn.disabled = !status.ok;
          execBtn.addEventListener("click", () => {
            const result = completeSupportOperation(chapterId);
            if (!result?.ok) showToast?.(result?.reason || "协作行动暂不可执行");
          });
          actions.appendChild(execBtn);
        }

        card.appendChild(actions);
        tracker.appendChild(card);
      });
    }

    function refreshSideQuests() {
      ensureGameplayState?.();
      const seed = Math.abs(state.world.regionX * 19 + state.world.regionY * 31 + state.story.chapterIndex * 11 + state.world.day);
      const defs = [
        { kind: "explore", title: "边境侦察：踏勘前哨路线", target: 3, rewardGold: 90, rewardExp: 65, faction: "wardens" },
        { kind: "combat", title: "清剿巡逻敌群", target: 2, rewardGold: 120, rewardExp: 80, faction: "freeguild" },
        { kind: "artifact", title: "回收遗物碎片", target: 2, rewardGold: 100, rewardExp: 70, faction: "arcanum" }
      ];
      state.sideQuests = defs.map((def, idx) => {
        const id = def.kind + "_" + state.story.chapterIndex + "_" + ((seed + idx) % 5);
        const old = (state.sideQuests || []).find((quest) => quest.id === id);
        return {
          id,
          kind: def.kind,
          title: def.title,
          target: def.target,
          progress: old ? old.progress : 0,
          completed: old ? old.completed : false,
          rewardGold: def.rewardGold,
          rewardExp: def.rewardExp,
          faction: def.faction
        };
      });
      showToast?.("支线任务已刷新");
      renderPanels?.();
    }

    function progressSideQuests(kind, amount = 1) {
      ensureGameplayState?.();
      for (const quest of state.sideQuests) {
        if (quest.completed || quest.kind !== kind) continue;
        quest.progress += amount;
        if (quest.progress >= quest.target) {
          quest.completed = true;
          state.storyRuntime.completedSideQuests += 1;
          addMissionMetric("side_quests_completed", 1);
          gain?.(quest.rewardExp, quest.rewardGold);
          applyReputation?.(quest.faction, 1);
          grantCompanionGrowth("支线协作", 16, 1, { toast: false });
          addJournal?.("委托完成: " + quest.title);
          showToast?.("委托完成: " + quest.title);
        }
      }
    }

    function recruitCompanion() {
      ensureGameplayState?.();
      ensureCompanionRosterState();
      if (state.player.gold < 80) return showToast?.("金币不足 80");
      if (state.party.roster.length >= 4) return showToast?.("队伍已满");

      const pool = COMPANION_ARCHETYPES.filter((entry) => !state.party.roster.some((mate) => mate.key === entry.key));
      if (!pool.length) return showToast?.("当前没有可招募同伴");

      const pick = pool[Math.floor(Math.random() * pool.length)];
      state.player.gold -= 80;
      const id = "cp_" + state.party.nextId++;
      const entry = ensureCompanionState({ ...pick, id, bond: 1, level: 1, exp: 0, nextExp: companionExpForLevel(1) });
      state.party.roster.push(entry);
      if (!state.party.activeId) state.party.activeId = id;

      addJournal?.("招募同伴: " + entry.name + " 加入了队伍，携带技能 " + entry.skillName);
      showToast?.("已招募: " + entry.name);
      renderAll?.();
    }

    return Object.freeze({
      activeCompanion,
      activeCompanionSummary,
      addMissionMetric,
      chapterMissionSummary,
      missionDone,
      missionMetricValue,
      chapterSupportSummary,
      checkChapterMissionRewards,
      companionPower,
      completeSupportOperation,
      ensureCompanionRosterState,
      grantCompanionGrowth,
      progressSideQuests,
      recruitCompanion,
      refreshSideQuests,
      renderSupportTracker,
      supportPrimaryBlockerText
    });
  }

  window.FantasyProgressionRuntimeModule = Object.freeze({
    createProgressionRuntimeModule
  });
})();
