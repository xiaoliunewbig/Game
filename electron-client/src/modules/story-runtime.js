(function initFantasyStoryRuntimeModule() {
  const {
    CHAPTERS,
    STORY_EVENTS,
    CHAPTER_STORYLINES,
    CHAPTER_ARCS
  } = window.FantasyGameData || {};

  function createStoryRuntimeModule(deps) {
    const {
      state,
      refs,
      showToast,
      renderAll,
      openModal,
      closeModal,
      ensureGameplayState,
      currentChapterId,
      chapter,
      addJournal,
      applyReputation,
      gain,
      addMissionMetric,
      progressSideQuests,
      refreshSideQuests,
      pushStoryCard,
      getPendingChapterDecision,
      buildDecisionImpactDetails,
      applyDecisionReward,
      buildChapterSummaryReport,
      evaluateEndingNarrative,
      openEndingReviewModal,
      getStoryProgressStatus
    } = deps || {};

    if (!state || !refs) {
      throw new Error("createStoryRuntimeModule requires state and refs.");
    }

    function checkDecisionRequirement(req) {
      if (!req) return { ok: true, reason: "" };
      if (req.min_shards && state.content.artifactShards < req.min_shards) {
        return { ok: false, reason: "\u9700\u8981\u788e\u7247 >= " + req.min_shards };
      }
      if (req.min_companions && (state.party?.roster?.length || 0) < req.min_companions) {
        return { ok: false, reason: "\u9700\u8981\u540c\u4f34\u6570 >= " + req.min_companions };
      }
      const repReq = req.min_reputation || {};
      for (const key of Object.keys(repReq)) {
        if ((state.factions?.[key] || 0) < repReq[key]) {
          return { ok: false, reason: "\u9700\u8981\u58f0\u671b " + key + " >= " + repReq[key] };
        }
      }
      return { ok: true, reason: "" };
    }

    function summarizeDecisionOption(opt) {
      const parts = [];
      if (opt.scene) parts.push(opt.scene);
      const reward = opt.reward || {};
      const rewardParts = [];
      if (reward.exp) rewardParts.push("EXP " + (reward.exp > 0 ? "+" : "") + reward.exp);
      if (reward.gold) rewardParts.push("\u91d1\u5e01 " + (reward.gold > 0 ? "+" : "") + reward.gold);
      if (reward.hp) rewardParts.push("HP " + (reward.hp > 0 ? "+" : "") + reward.hp);
      if (reward.mp) rewardParts.push("MP " + (reward.mp > 0 ? "+" : "") + reward.mp);
      if (reward.shards) rewardParts.push("\u788e\u7247 " + (reward.shards > 0 ? "+" : "") + reward.shards);
      if (reward.atk) rewardParts.push("ATK +" + reward.atk);
      if (reward.def) rewardParts.push("DEF +" + reward.def);
      if (rewardParts.length) parts.push("\u7ed3\u679c: " + rewardParts.join(", "));
      return parts.join("\n");
    }

    function getChapterArcState(chapterId = currentChapterId()) {
      ensureGameplayState?.();
      const arc = state.storyRuntime.arc;
      if (!arc[chapterId]) arc[chapterId] = { index: 0, completed: false };
      return arc[chapterId];
    }

    function getCurrentArcNode(chapterId = currentChapterId()) {
      const nodes = CHAPTER_ARCS[chapterId] || [];
      const arcState = getChapterArcState(chapterId);
      if (arcState.index >= nodes.length) {
        arcState.completed = true;
        return null;
      }
      return nodes[arcState.index];
    }

    function getChapterEventText(index) {
      const chapterId = currentChapterId();
      const pool = CHAPTER_STORYLINES[chapterId] || STORY_EVENTS || [];
      if (!pool.length) return "";
      return pool[index % pool.length];
    }

    function applyChapterRouteBonus() {
      const chapterId = currentChapterId();
      if (chapterId === "camp") {
        if (state.story.route === "order") {
          state.player.maxHp += 4;
          state.player.hp = Math.min(state.player.maxHp, state.player.hp + 4);
        } else if (state.story.route === "chaos") {
          state.player.gold += 40;
        } else {
          state.player.exp += 25;
        }
      } else if (chapterId === "forest") {
        if (state.story.route === "order") {
          applyReputation("wardens", 1);
        } else if (state.story.route === "chaos") {
          applyReputation("freeguild", 1);
          state.player.hp = Math.max(1, state.player.hp - 4);
        } else {
          applyReputation("arcanum", 1);
          state.player.mp = Math.min(state.player.maxMp, state.player.mp + 8);
        }
      } else if (chapterId === "rift") {
        if (state.story.route === "order") state.player.defenseBonus += 1;
        if (state.story.route === "chaos") state.player.attackBonus += 1;
        if (state.story.route === "balanced") {
          state.player.attackBonus += 1;
          state.player.defenseBonus += 1;
        }
      }
    }

    function applyArcReward(reward) {
      if (!reward) return;
      if (reward.exp || reward.gold) gain(reward.exp || 0, reward.gold || 0);
      if (reward.shards) state.content.artifactShards += reward.shards;
      const rep = reward.rep || {};
      if (typeof rep.wardens === "number") applyReputation("wardens", rep.wardens);
      if (typeof rep.arcanum === "number") applyReputation("arcanum", rep.arcanum);
      if (typeof rep.freeguild === "number") applyReputation("freeguild", rep.freeguild);
      const progress = reward.progress || {};
      if (progress.explore) state.story.progress.explore += progress.explore;
      if (progress.artifact) state.story.progress.artifact += progress.artifact;
      if (progress.boss) state.story.progress.boss += progress.boss;
    }

    function triggerChapterArcStep() {
      const chapterId = currentChapterId();
      const nodes = CHAPTER_ARCS[chapterId] || [];
      if (!nodes.length) return null;

      const arcState = getChapterArcState(chapterId);
      const node = getCurrentArcNode(chapterId);
      if (!node) return null;

      applyArcReward(node.reward || {});
      addMissionMetric?.("arc_steps", 1, chapterId);
      arcState.index += 1;
      if (arcState.index >= nodes.length) arcState.completed = true;
      state.storyRuntime.beats += 1;

      addJournal?.("\u4e3b\u7ebf\u63a8\u8fdb: " + node.title + " - " + node.desc);
      pushStoryCard?.("arc", node.title, node.desc);
      return {
        node,
        remaining: Math.max(0, nodes.length - arcState.index),
        completed: Boolean(arcState.completed)
      };
    }

    function chooseStoryDecision(decision, option) {
      ensureGameplayState?.();
      const gate = checkDecisionRequirement(option.require);
      if (!gate.ok) return showToast?.(gate.reason);
      state.storyRuntime.decisions[decision.id] = option.id;
      addMissionMetric?.("decisions_committed", 1);
      const rep = option.rep || {};
      if (typeof rep.wardens === "number") applyReputation("wardens", rep.wardens);
      if (typeof rep.arcanum === "number") applyReputation("arcanum", rep.arcanum);
      if (typeof rep.freeguild === "number") applyReputation("freeguild", rep.freeguild);
      applyDecisionReward?.(option.reward || {});
      addJournal?.("\u6295\u62e9: " + decision.title + " -> " + option.label);
      const decisionImpact = buildDecisionImpactDetails?.(decision.id, option.id);
      const decisionBody = option.label + " | " + (option.scene || "") + (decisionImpact ? ("\n\u5f71\u54cd: " + decisionImpact) : "");
      pushStoryCard?.("decision", decision.title, decisionBody);
      closeModal?.("modalStoryDecision");
      renderAll?.();
    }

    function openStoryDecisionModal() {
      const decision = getPendingChapterDecision?.();
      if (!decision) return showToast?.("\u5f53\u524d\u7ae0\u8282\u6682\u65e0\u5f85\u5904\u7406\u6295\u62e9");
      if (refs.storyDecisionTitle) refs.storyDecisionTitle.textContent = decision.title;
      if (refs.storyDecisionDesc) refs.storyDecisionDesc.textContent = decision.desc;
      if (refs.storyDecisionOptions) {
        refs.storyDecisionOptions.innerHTML = "";
        decision.options.forEach((opt) => {
          const wrap = document.createElement("div");
          wrap.className = "quest-item";
          const btn = document.createElement("button");
          btn.className = "side-btn";
          btn.textContent = opt.label;
          const gate = checkDecisionRequirement(opt.require);
          if (!gate.ok) {
            btn.disabled = true;
            btn.title = gate.reason;
          }
          btn.addEventListener("click", () => chooseStoryDecision(decision, opt));
          const note = document.createElement("div");
          note.className = "snapshot-title";
          const prefix = gate.ok ? "" : "\u672a\u89e3\u9501: " + gate.reason + "\n";
          note.textContent = prefix + summarizeDecisionOption(opt);
          wrap.appendChild(btn);
          wrap.appendChild(note);
          refs.storyDecisionOptions.appendChild(wrap);
        });
      }
      openModal?.("modalStoryDecision");
    }

    function triggerStoryEvent() {
      ensureGameplayState?.();
      const arcResult = triggerChapterArcStep();
      addMissionMetric?.("story_events", 1);

      if (arcResult) {
        progressSideQuests?.("explore", 1);
        if (arcResult.completed) {
          showToast?.("\u4e3b\u7ebf\u8282\u70b9\u5b8c\u6210: " + arcResult.node.title);
        } else {
          showToast?.("\u4e3b\u7ebf\u63a8\u8fdb: " + arcResult.node.title);
        }
        if (getPendingChapterDecision?.()) {
          showToast?.("\u51fa\u73b0\u65b0\u7684\u5267\u60c5\u6295\u62e9\uff0c\u53ef\u70b9\u51fb\u3010\u5267\u60c5\u6295\u62e9\u3011\u3002");
        }
        renderAll?.();
        return;
      }

      const idx = (state.storyRuntime.beats + state.story.chapterIndex) % 9;
      const eventText = getChapterEventText(idx);
      state.storyRuntime.beats += 1;

      if (state.story.route === "order") {
        applyReputation("wardens", 1);
        gain(18, 16);
      } else if (state.story.route === "chaos") {
        applyReputation("freeguild", 1);
        gain(16, 28);
        state.player.hp = Math.max(1, state.player.hp - 4);
      } else {
        applyReputation("arcanum", 1);
        gain(22, 18);
        state.player.mp = Math.min(state.player.maxMp, state.player.mp + 4);
      }

      if (currentChapterId() === "camp") state.story.progress.explore += 1;
      if (currentChapterId() === "forest") state.story.progress.artifact += 1;

      progressSideQuests?.("explore", 1);
      addJournal?.("\u5267\u60c5\u63a8\u8fdb: " + eventText);
      pushStoryCard?.("event", "\u5267\u60c5\u8282\u62cd", eventText);
      showToast?.("\u5267\u60c5\u4e8b\u4ef6\u5df2\u63a8\u8fdb\uff0c\u8bf7\u67e5\u770b\u4efb\u52a1\u6307\u5f15\u3002");
      renderAll?.();
    }

    function triggerCombatEvent() {
      ensureGameplayState?.();
      const enemy = 8 + Math.floor(Math.random() * 18);
      const mitigation = Math.floor((state.player.baseDefense + state.player.defenseBonus) * 0.4);
      const damage = Math.max(1, enemy - mitigation);
      state.player.hp = Math.max(1, state.player.hp - damage);
      const reward = 24 + Math.floor(Math.random() * 24);
      gain(28, reward);
      addMissionMetric?.("combat_wins", 1);
      progressSideQuests?.("combat", 1);
      state.story.progress.explore += 1;
      addJournal?.("\u906d\u9047\u6218\u80dc\u5229\uff0c\u53d7\u5230\u4f24\u5bb3 " + damage + " \u70b9\u3002");
      showToast?.("\u6218\u6597\u4e8b\u4ef6\u5904\u7406\u5b8c\u6210");
      renderAll?.();
    }

    function exploreArea() {
      ensureGameplayState?.();
      addMissionMetric?.("explore_steps", 1);
      state.world.day += 1;

      const arcState = getChapterArcState(currentChapterId());
      if (!arcState.completed) {
        const arcResult = triggerChapterArcStep();
        if (arcResult) {
          progressSideQuests?.("explore", 1);
          if (arcResult.completed) {
            showToast?.("\u63a2\u7d22\u63a8\u8fdb\u4e86\u4e3b\u7ebf\uff1a" + arcResult.node.title + "\uff08\u672c\u6bb5\u5df2\u5b8c\u6210\uff09");
          } else {
            showToast?.("\u63a2\u7d22\u63a8\u8fdb\u4e86\u4e3b\u7ebf\uff1a" + arcResult.node.title);
          }
          if (getPendingChapterDecision?.()) {
            showToast?.("\u51fa\u73b0\u65b0\u7684\u5267\u60c5\u6295\u62e9\uff0c\u8bf7\u70b9\u51fb\u3010\u5267\u60c5\u6295\u62e9\u3011\u3002");
          }
          renderAll?.();
          return;
        }
      }

      const roll = Math.random();
      if (roll < 0.4) {
        const hurt = 6 + Math.floor(Math.random() * 12);
        state.player.hp = Math.max(1, state.player.hp - hurt);
        state.story.progress.explore += 1;
        gain(18, 20);
        progressSideQuests?.("combat", 1);
        addJournal?.("\u63a2\u7d22\u906d\u9047\u4f0f\u51fb\uff0cHP -" + hurt);
      } else if (roll < 0.78) {
        const shard = 1 + Math.floor(Math.random() * 2);
        state.content.artifactShards += shard;
        addMissionMetric?.("artifact_found", 1);
        state.story.progress.artifact += 1;
        gain(14, 28);
        progressSideQuests?.("artifact", 1);
        addJournal?.("\u56de\u6536\u9057\u7269\u788e\u7247 +" + shard);
      } else {
        state.player.potions += 1;
        gain(24, 36);
        progressSideQuests?.("explore", 1);
        addJournal?.("\u53d1\u73b0\u8865\u7ed9\u7bb1\uff0c\u836f\u6c34 +1");
      }
      showToast?.("\u63a2\u7d22\u5b8c\u6210\uff0c\u5267\u60c5\u5df2\u63a8\u8fdb\u3002");
      renderAll?.();
    }

    function commitChoice() {
      ensureGameplayState?.();
      state.story.choice = refs.storyChoiceSelect?.value || "balanced";
      state.story.route = state.story.choice;
      if (state.story.route === "order") applyReputation("wardens", 1);
      if (state.story.route === "balanced") applyReputation("arcanum", 1);
      if (state.story.route === "chaos") applyReputation("freeguild", 1);
      addJournal?.("\u5df2\u786e\u8ba4\u5267\u60c5\u8def\u7ebf: " + state.story.route);
      showToast?.("\u5267\u60c5\u8def\u7ebf\u5df2\u786e\u8ba4\u3002");
      renderAll?.();
    }

    function advanceChapter() {
      ensureGameplayState?.();
      const currentChapter = chapter();
      const status = getStoryProgressStatus?.(currentChapter);
      if (status && !status.canAdvance) return showToast?.(status.blockers[0]);

      const chapterReport = buildChapterSummaryReport?.(currentChapter);
      state.storyRuntime.chapterReports.push(chapterReport);
      applyChapterRouteBonus();
      gain(currentChapter.exp, currentChapter.gold);
      pushStoryCard?.(
        "chapter",
        "\u7ae0\u8282\u901a\u5173: " + currentChapter.name,
        "\u8def\u7ebf " + (chapterReport?.route || "balanced") + "\uff0c\u7b49\u7ea7 " + chapterReport?.level + "\uff0c\u58f0\u671b\u5feb\u7167\u5df2\u8bb0\u5f55\u3002"
      );

      if (state.story.chapterIndex < CHAPTERS.length - 1) {
        state.story.chapterIndex += 1;
        state.world.chapter = chapter().name;
        refreshSideQuests?.();
        addJournal?.("\u7ae0\u8282\u63a8\u8fdb\u81f3 " + state.world.chapter);
        showToast?.("\u8fdb\u5165\u7ae0\u8282: " + state.world.chapter);
      } else {
        const ending = evaluateEndingNarrative?.();
        state.storyRuntime.ending = ending;
        addJournal?.(ending);
        showToast?.("\u5df2\u5b8c\u6210\u5168\u90e8\u7ae0\u8282\uff0c\u53ef\u5728\u7ed3\u5c40\u590d\u76d8\u67e5\u770b\u603b\u7ed3\u3002");
        openEndingReviewModal?.();
      }
      renderAll?.();
    }

    return Object.freeze({
      advanceChapter,
      checkDecisionRequirement,
      commitChoice,
      exploreArea,
      getChapterArcState,
      getCurrentArcNode,
      openStoryDecisionModal,
      triggerCombatEvent,
      triggerStoryEvent
    });
  }

  window.FantasyStoryRuntimeModule = Object.freeze({
    createStoryRuntimeModule
  });
})();