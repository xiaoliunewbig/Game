// Chapter rumor extension for NPC and story guide systems.

(() => {
  if (typeof currentChapterId !== "function" || typeof renderAll !== "function") return;

  const RUMOR_STYLE_ID = "chapter-rumor-style";
  const CHAPTER_RUMORS = {
    camp: [
      {
        id: "camp_signal_smoke",
        npcKey: "npc_watch_lieutenant",
        title: "\u5317\u7ebf\u8bef\u70df",
        desc: "\u57c3\u4e3d\u62c9\u6000\u7591\u5317\u4fa7\u54e8\u70b9\u51fa\u73b0\u4e86\u4f2a\u9020\u70fd\u706b\uff0c\u4f60\u9700\u8981\u5148\u8e0f\u52d8\u5730\u6807\uff0c\u518d\u4e0e\u5979\u6838\u5bf9\u706f\u5e8f\u3002",
        require: { trust: 2, poiVisited: 1 },
        reward: { exp: 18, gold: 24, rep: { wardens: 1 }, progress: { explore: 1 } },
        journal: "\u4f60\u4e0e\u57c3\u4e3d\u62c9\u4fee\u6b63\u4e86\u6668\u54e8\u706f\u5e8f\uff0c\u8425\u5730\u8bef\u62a5\u6682\u65f6\u5e73\u606f\u3002"
      },
      {
        id: "camp_medic_convoy",
        npcKey: "npc_field_medic",
        title: "\u6d41\u52a8\u6551\u62a4\u7ebf",
        desc: "\u5b89\u5a1c\u5e0c\u671b\u5efa\u7acb\u591c\u95f4\u6551\u62a4\u7ebf\uff0c\u9700\u8981\u4f60\u5148\u5e26\u4e0a\u4e00\u540d\u540c\u884c\u4f19\u4f34\u3002",
        require: { relationUnlocked: 1, requireCompanion: true },
        reward: { exp: 22, hp: 14, def: 1, rep: { wardens: 1 } },
        journal: "\u4f60\u534f\u52a9\u5b89\u5a1c\u642d\u8d77\u4e86\u591c\u95f4\u8f6c\u8fd0\u7ebf\uff0c\u8425\u5730\u533b\u7597\u538b\u529b\u51cf\u8f7b\u4e86\u3002"
      }
    ],
    forest: [
      {
        id: "forest_rune_echo",
        npcKey: "npc_rune_scholar",
        title: "\u6797\u6d77\u56de\u54cd\u8c31",
        desc: "\u7ef4\u5c14\u53d1\u73b0\u9057\u7269\u8109\u51b2\u4f1a\u5728\u53e4\u6811\u4e4b\u95f4\u6298\u8fd4\uff0c\u9700\u8981\u4f60\u5148\u63a8\u8fdb\u4e00\u9879\u7ae0\u8282\u4efb\u52a1\u3002",
        require: { trust: 2, missionDone: 1 },
        reward: { exp: 24, shards: 1, rep: { arcanum: 1 }, progress: { artifact: 1 } },
        journal: "\u4f60\u4e0e\u7ef4\u5c14\u7ed8\u51fa\u4e86\u6797\u6d77\u56de\u54cd\u8c31\uff0c\u9057\u7269\u641c\u5bfb\u6548\u7387\u663e\u8457\u63d0\u9ad8\u3002"
      },
      {
        id: "forest_predator_route",
        npcKey: "npc_beast_hunter",
        title: "\u591c\u730e\u8fc1\u5f99\u7ebf",
        desc: "\u51ef\u6069\u60f3\u622a\u65ad\u5f71\u517d\u8fc1\u5f99\u7ebf\uff0c\u9700\u8981\u4f60\u5148\u5b8c\u6574\u63a8\u8fdb\u4e00\u6761 NPC \u5267\u60c5\u7ebf\u3002",
        require: { chainCompleted: 1, poiVisited: 1 },
        reward: { exp: 26, gold: 30, atk: 1 },
        journal: "\u4f60\u4e0e\u51ef\u6069\u5207\u65ad\u4e86\u5f71\u517d\u591c\u884c\u8def\u7ebf\uff0c\u68ee\u6797\u8fb9\u7ebf\u7ec8\u4e8e\u7a33\u5b9a\u4e0b\u6765\u3002"
      }
    ],
    rift: [
      {
        id: "rift_anchor_drift",
        npcKey: "npc_front_engineer",
        title: "\u6f02\u79fb\u951a\u70b9",
        desc: "\u6258\u6bd4\u6d4b\u5230\u951a\u70b9\u76f8\u4f4d\u5f00\u59cb\u6f02\u79fb\uff0c\u9700\u8981\u4f60\u5148\u5b8c\u6210\u4e24\u9879\u7ae0\u8282\u4efb\u52a1\u3002",
        require: { trust: 2, missionDone: 2 },
        reward: { exp: 28, shards: 1, mp: 12, rep: { arcanum: 1 }, progress: { boss: 1 } },
        journal: "\u4f60\u4e0e\u6258\u6bd4\u7a33\u5b9a\u4e86\u6f02\u79fb\u951a\u70b9\uff0c\u4e3a\u88c2\u9699\u524d\u7ebf\u4e89\u53d6\u5230\u5b9d\u8d35\u65f6\u95f4\u3002"
      },
      {
        id: "rift_shield_rotation",
        npcKey: "npc_shield_captain",
        title: "\u76fe\u5899\u6362\u5217",
        desc: "\u6258\u4f26\u51c6\u5907\u91cd\u7f16\u7b2c\u4e09\u9632\u7ebf\u7684\u6362\u5217\u8282\u594f\uff0c\u9700\u8981\u4f60\u5b8c\u6210\u4e24\u6761 NPC \u5267\u60c5\u7ebf\u5e76\u5e26\u4e0a\u4e00\u540d\u540c\u4f34\u3002",
        require: { chainCompleted: 2, requireCompanion: true },
        reward: { exp: 30, def: 1, gold: 24, rep: { wardens: 1 } },
        journal: "\u4f60\u534f\u52a9\u6258\u4f26\u91cd\u7f16\u4e86\u76fe\u5899\u6362\u5217\u8282\u594f\uff0c\u7b2c\u4e09\u9632\u7ebf\u627f\u538b\u80fd\u529b\u660e\u663e\u63d0\u5347\u3002"
      }
    ]
  };

  function injectRumorStyles() {
    if (document.getElementById(RUMOR_STYLE_ID)) return;
    const style = document.createElement("style");
    style.id = RUMOR_STYLE_ID;
    style.textContent = [
      ".rumor-board { display:grid; gap:8px; margin:8px 0 10px; }",
      ".rumor-card { border:1px solid #6d5630; border-radius:12px; padding:10px; background:linear-gradient(180deg, rgba(38,27,18,.94), rgba(19,14,10,.94)); }",
      ".rumor-card.ready { box-shadow:0 0 0 1px rgba(214,170,96,.22); }",
      ".rumor-card.done { border-color:#54734a; background:linear-gradient(180deg, rgba(24,35,22,.94), rgba(14,20,12,.94)); }",
      ".rumor-head { display:flex; align-items:flex-start; justify-content:space-between; gap:10px; margin-bottom:6px; }",
      ".rumor-title { color:#fff2d0; font-size:13px; font-weight:700; }",
      ".rumor-meta { color:#ccb795; font-size:11px; margin-top:3px; }",
      ".rumor-badge { border-radius:999px; padding:3px 8px; font-size:11px; color:#ffe6b7; border:1px solid rgba(211,168,95,.35); background:rgba(177,137,74,.18); }",
      ".rumor-desc { color:#e1d3bb; font-size:12px; line-height:1.5; margin-bottom:8px; }",
      ".rumor-note { color:#ffe2a8; font-size:11px; line-height:1.45; }"
    ].join("\n");
    document.head.appendChild(style);
  }

  function ensureRumorState() {
    if (!state.npcs) state.npcs = {};
    if (!Array.isArray(state.npcs.rumorsCompleted)) state.npcs.rumorsCompleted = [];
  }

  function rumorStateId(chapterId, rumorId) {
    return String(chapterId) + "::rumor::" + String(rumorId);
  }

  function rumorDefs(chapterId = currentChapterId()) {
    return CHAPTER_RUMORS[chapterId] || [];
  }

  function rumorNpc(npcKey, chapterId = currentChapterId()) {
    ensureRumorState();
    return (state.npcs.activeMap || []).find((npc) => npc.key === npcKey && String(npc.chapterId || currentChapterId()) === String(chapterId)) || null;
  }

  function hasRumorCompleted(chapterId, rumorId) {
    ensureRumorState();
    return state.npcs.rumorsCompleted.includes(rumorStateId(chapterId, rumorId));
  }

  function nextRumorForNpc(npc, chapterId = currentChapterId()) {
    if (!npc) return null;
    return rumorDefs(chapterId).find((item) => item.npcKey === npc.key && !hasRumorCompleted(chapterId, item.id)) || null;
  }

  function rumorStatus(def, chapterId = currentChapterId()) {
    const npc = rumorNpc(def.npcKey, chapterId);
    const trust = npc ? npcTrustValue(npc.id) : 0;
    const relation = npc ? npcRelationshipEventStatus(npc) : { unlocked: 0, total: 0 };
    const chain = npc ? npcChainProgress(npc) : { doneCount: 0 };
    const poi = typeof regionPoiSummary === "function" ? regionPoiSummary(chapterId) : { done: 0, total: 0 };
    const mission = typeof chapterMissionSummary === "function" ? chapterMissionSummary(chapterId) : { done: 0, total: 0 };
    const sideDone = Number(state.storyRuntime?.completedSideQuests || 0);
    const mate = typeof activeCompanion === "function" ? activeCompanion() : null;
    const req = def.require || {};
    const blockers = [];

    if (req.trust && trust < req.trust) blockers.push("\u597d\u611f " + trust + "/" + req.trust);
    if (req.relationUnlocked && relation.unlocked < req.relationUnlocked) blockers.push("\u5173\u7cfb\u4e8b\u4ef6 " + relation.unlocked + "/" + req.relationUnlocked);
    if (req.chainCompleted && Number(chain.doneCount || 0) < Number(req.chainCompleted)) blockers.push("NPC \u5267\u60c5\u7ebf " + Number(chain.doneCount || 0) + "/" + req.chainCompleted);
    if (req.poiVisited && Number(poi.done || 0) < Number(req.poiVisited)) blockers.push("\u533a\u57df\u5730\u6807 " + Number(poi.done || 0) + "/" + req.poiVisited);
    if (req.missionDone && Number(mission.done || 0) < Number(req.missionDone)) blockers.push("\u7ae0\u8282\u4efb\u52a1 " + Number(mission.done || 0) + "/" + req.missionDone);
    if (req.sideQuests && sideDone < Number(req.sideQuests)) blockers.push("\u652f\u7ebf\u4efb\u52a1 " + sideDone + "/" + req.sideQuests);
    if (req.requireCompanion && !mate) blockers.push("\u9700\u8981\u540c\u884c\u4f19\u4f34");

    return { npc, trust, relation, chain, poi, mission, sideDone, mate, blockers, ok: blockers.length === 0, completed: hasRumorCompleted(chapterId, def.id) };
  }

  function rumorSummary(chapterId = currentChapterId()) {
    ensureRumorState();
    const defs = rumorDefs(chapterId);
    const completed = defs.filter((item) => hasRumorCompleted(chapterId, item.id)).length;
    const current = defs.find((item) => !hasRumorCompleted(chapterId, item.id)) || null;
    return { total: defs.length, completed, remaining: Math.max(0, defs.length - completed), defs, current, currentStatus: current ? rumorStatus(current, chapterId) : null };
  }

  function rumorBlockerText(def, status) {
    return status?.blockers?.[0] || (def?.desc || "\u7ee7\u7eed\u4e0e\u7ae0\u8282 NPC \u4e92\u52a8");
  }

  function applyRumorReward(def, chapterId = currentChapterId()) {
    if (!def?.reward) return;
    applyDecisionReward(def.reward);
    const rep = def.reward.rep || {};
    if (typeof rep.wardens === "number") applyReputation("wardens", rep.wardens);
    if (typeof rep.arcanum === "number") applyReputation("arcanum", rep.arcanum);
    if (typeof rep.freeguild === "number") applyReputation("freeguild", rep.freeguild);
    if (typeof addMissionMetric === "function") addMissionMetric("story_events", 1, chapterId);
  }

  function triggerRumor(npc, source = "\u4ea4\u8c08") {
    if (!npc) return false;
    ensureRumorState();
    const chapterId = npc.chapterId || currentChapterId();
    const def = nextRumorForNpc(npc, chapterId);
    if (!def || hasRumorCompleted(chapterId, def.id)) return false;

    const status = rumorStatus(def, chapterId);
    if (!status.ok) return false;

    state.npcs.rumorsCompleted.push(rumorStateId(chapterId, def.id));
    applyRumorReward(def, chapterId);

    const rewardText = typeof formatRewardInline === "function" ? formatRewardInline(def.reward || {}) : "";
    addJournal(def.journal || ("\u7ae0\u8282\u4f20\u95fb\u63a8\u8fdb\uff1a" + def.title));
    pushStoryCard(
      "event",
      "\u7ae0\u8282\u4f20\u95fb\uff1a" + def.title,
      (def.desc || "") + (rewardText ? ("\n\u5956\u52b1\uff1a" + rewardText) : "") + (source ? ("\n\u89e6\u53d1\u65b9\u5f0f\uff1a" + source) : "")
    );
    showToast("\u4f20\u95fb\u63a8\u8fdb\uff1a" + def.title);
    return true;
  }

  function upsertLine(el, prefix, line) {
    if (!el) return;
    const lines = String(el.textContent || "").split("\n");
    const index = lines.findIndex((item) => item.startsWith(prefix));
    if (index >= 0) lines[index] = line;
    else lines.push(line);
    el.textContent = lines.filter(Boolean).join("\n");
  }

  function rumorSummaryLine(summary) {
    if (!summary.total) return "\u4f20\u95fb\uff1a\u5f53\u524d\u7ae0\u8282\u6682\u65e0\u4f20\u95fb";
    if (!summary.current) return "\u4f20\u95fb\uff1a\u7ae0\u8282\u4f20\u95fb\u5df2\u5b8c\u6210 " + summary.completed + "/" + summary.total;
    return "\u4f20\u95fb\uff1a" + summary.current.title + (summary.currentStatus?.ok ? " | \u53ef\u524d\u5f80\u5bf9\u5e94 NPC \u63a8\u8fdb" : (" | \u4ecd\u9700\uff1a" + rumorBlockerText(summary.current, summary.currentStatus)));
  }

  function ensureRumorBoard() {
    if (!refs?.npcList || !refs.npcList.parentElement) return null;
    let board = document.getElementById("npcRumorBoard");
    if (!board) {
      board = document.createElement("div");
      board.id = "npcRumorBoard";
      board.className = "rumor-board";
      refs.npcList.parentElement.insertBefore(board, refs.npcList);
    }
    return board;
  }

  function renderRumorBoard() {
    const board = ensureRumorBoard();
    if (!board) return;

    const summary = rumorSummary(currentChapterId());
    if (!summary.total) {
      board.innerHTML = "";
      return;
    }

    board.innerHTML = summary.defs.map((def) => {
      const status = rumorStatus(def, currentChapterId());
      const npc = status.npc;
      const badge = status.completed ? "\u5df2\u5b8c\u6210" : (status.ok ? "\u53ef\u89e6\u53d1" : "\u8ffd\u8e2a\u4e2d");
      const note = status.completed
        ? "\u8fd9\u6761\u4f20\u95fb\u5df2\u7ecf\u5199\u5165\u65e5\u5fd7\u548c\u5267\u60c5\u5361\u3002"
        : (status.ok ? "\u4e0e\u5bf9\u5e94 NPC \u4ea4\u8c08\u5373\u53ef\u89e6\u53d1\u3002" : ("\u4ecd\u9700\uff1a" + rumorBlockerText(def, status)));
      return "<div class='rumor-card" + (status.completed ? " done" : (status.ok ? " ready" : "")) + "'><div class='rumor-head'><div><div class='rumor-title'>" + def.title + "</div><div class='rumor-meta'>\u5173\u8054 NPC\uff1a" + (npc?.name || def.npcKey) + "</div></div><span class='rumor-badge'>" + badge + "</span></div><div class='rumor-desc'>" + def.desc + "</div><div class='rumor-note'>" + note + "</div></div>";
    }).join("");
  }

  function openCurrentRumorNpc() {
    const summary = rumorSummary(currentChapterId());
    const npc = summary.current ? rumorNpc(summary.current.npcKey, currentChapterId()) : null;
    if (!npc) return showToast("\u5f53\u524d\u5730\u56fe\u6ca1\u6709\u5bf9\u5e94\u7684\u4f20\u95fb NPC");
    openNpcDialogById(npc.id);
  }

  function patchRuntime() {
    const originalRenderPanels = renderPanels;
    renderPanels = function() {
      originalRenderPanels();
      const summary = rumorSummary(currentChapterId());
      if (refs?.projectSummary && summary.total) {
        upsertLine(refs.projectSummary, "\u4f20\u95fb\uff1a", rumorSummaryLine(summary));
      }
    };

    const originalRenderNpcPanel = renderNpcPanel;
    renderNpcPanel = function() {
      originalRenderNpcPanel();
      const summary = rumorSummary(currentChapterId());
      if (refs?.npcSummary && summary.total) {
        upsertLine(refs.npcSummary, "\u4f20\u95fb\uff1a", rumorSummaryLine(summary));
      }
      renderRumorBoard();
    };

    const originalRenderNpcDialog = renderNpcDialog;
    renderNpcDialog = function() {
      originalRenderNpcDialog();
      const npc = typeof findNpcById === "function" ? findNpcById(activeNpcDialogId || state.npcs?.selectedId || "") : null;
      const rumor = nextRumorForNpc(npc, currentChapterId());
      const status = rumor ? rumorStatus(rumor, currentChapterId()) : null;
      const line = rumor
        ? ("\u7ae0\u8282\u4f20\u95fb\uff1a" + rumor.title + (status?.ok ? "\uff08\u53ef\u89e6\u53d1\uff09" : "\uff08" + rumorBlockerText(rumor, status) + "\uff09"))
        : "\u7ae0\u8282\u4f20\u95fb\uff1a\u5f53\u524d\u6ca1\u6709\u5f85\u63a8\u8fdb\u7ebf\u7d22";
      if (refs?.npcDialogBody) upsertLine(refs.npcDialogBody, "\u7ae0\u8282\u4f20\u95fb\uff1a", line);
      if (refs?.npcActionSummary) upsertLine(refs.npcActionSummary, "\u7ae0\u8282\u4f20\u95fb\uff1a", line);
    };

    const originalNpcActionTalk = npcActionTalk;
    npcActionTalk = function() {
      const npcId = activeNpcDialogId;
      const result = originalNpcActionTalk();
      const npc = typeof findNpcById === "function" ? findNpcById(npcId) : null;
      if (npc && triggerRumor(npc, "\u4ea4\u8c08")) {
        renderAll();
        renderNpcDialog();
      }
      return result;
    };
  }

  injectRumorStyles();
  ensureRumorState();
  patchRuntime();

  window.rumorSummary = rumorSummary;
  window.rumorBlockerText = rumorBlockerText;
  window.openCurrentRumorNpc = openCurrentRumorNpc;
  window.FantasyRumorModule = Object.freeze({
    rumorDefs,
    rumorSummary,
    rumorBlockerText,
    renderRumorBoard,
    openCurrentRumorNpc,
    triggerRumor
  });

  queueMicrotask(() => {
    try {
      renderAll();
      if (!document.getElementById("modalNpcDialog")?.classList.contains("hidden")) renderNpcDialog();
    } catch (error) {
      console.warn("[rumor patch]", error);
    }
  });
})();
