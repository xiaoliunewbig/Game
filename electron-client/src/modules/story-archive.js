// Story replay, summary, ending review, and decision archive helpers.

function buildEndingReviewPayload() {
  ensureGameplayState();
  const rep = state.factions || { wardens: 0, arcanum: 0, freeguild: 0 };
  const companions = (state.party?.roster || []).map((m) => ({ name: m.name, role: m.role, bond: m.bond }));
  const payload = {
    player: {
      name: state.player.name,
      profession: state.player.profession,
      level: state.player.level,
      hp: state.player.hp,
      mp: state.player.mp,
      gold: state.player.gold
    },
    story: {
      chapter: state.world.chapter,
      route: state.story.route,
      beats: state.storyRuntime.beats || 0,
      sideQuestsCompleted: state.storyRuntime.completedSideQuests || 0,
      ending: state.storyRuntime.ending || "\u672a\u5b8c\u6210"
    },
    factions: rep,
    relics: {
      forged: state.content.forgedRelics,
      shards: state.content.artifactShards
    },
    talents: state.talents,
    companions: companions
  };
  return payload;
}

function renderEndingReviewContent() {
  if (!refs.endingReviewContent) return;
  const p = buildEndingReviewPayload();
  const rep = p.factions;
  const lines = [];
  lines.push("=== \u7ec8\u7ae0\u590d\u76d8 ===");
  lines.push("\u4e3b\u89d2: " + p.player.name + " (" + p.player.profession + ") Lv." + p.player.level);
  lines.push("\u8def\u7ebf: " + p.story.route + " | \u7ae0\u8282: " + p.story.chapter);
  lines.push("\u5267\u60c5\u8282\u62cd: " + p.story.beats + " | \u652f\u7ebf\u5b8c\u6210: " + p.story.sideQuestsCompleted);
  lines.push("\u58f0\u671b -> \u5b88\u671b\u8005 " + rep.wardens + " | \u5965\u672f\u8bae\u4f1a " + rep.arcanum + " | \u81ea\u7531\u884c\u4f1a " + rep.freeguild);
  lines.push("\u9057\u7269 -> \u5df2\u953b\u9020 " + p.relics.forged + " | \u788e\u7247 " + p.relics.shards);
  lines.push("\u5929\u8d4b -> \u8f93\u51fa " + p.talents.offense + " | \u9632\u5fa1 " + p.talents.guard + " | \u652f\u63f4 " + p.talents.support + " (\u672a\u5206\u914d " + p.talents.points + ")");
  if (p.companions.length) {
    lines.push("\u540c\u4f34:");
    for (const m of p.companions) lines.push("- " + m.name + " / " + m.role + " / \u7f81\u7eca " + m.bond);
  } else {
    lines.push("\u540c\u4f34: \u65e0");
  }
  lines.push("\u7ed3\u5c40: " + p.story.ending);
  refs.endingReviewContent.textContent = lines.join("\n");
}

function openEndingReviewModal() {
  renderEndingReviewContent();
  openModal("modalEndingReview");
}

function downloadTextFile(filename, text) {
  const blob = new Blob([text], { type: "text/plain;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = filename;
  a.click();
  URL.revokeObjectURL(url);
}

function downloadJsonFile(filename, value) {
  const blob = new Blob([JSON.stringify(value, null, 2)], { type: "application/json" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = filename;
  a.click();
  URL.revokeObjectURL(url);
}

function exportStoryReplay(asJson) {
  ensureGameplayState();
  const stamp = Date.now();
  const filter = refs.selectStoryReplayType?.value || "all";
  const all = state.storyRuntime?.storyCards || [];
  const list = filter === "all" ? all : all.filter((x) => x.type === filter);
  if (asJson) {
    downloadJsonFile("story_replay_" + filter + "_" + stamp + ".json", list);
    return;
  }
  renderStoryReplay();
  const text = refs.storyReplayList?.textContent || "\u6682\u65e0\u53ef\u7528\u6570\u636e\u3002";
  downloadTextFile("story_replay_" + filter + "_" + stamp + ".txt", text);
}

function exportChapterSummary(asJson) {
  ensureGameplayState();
  const stamp = Date.now();
  const reports = state.storyRuntime?.chapterReports || [];
  if (asJson) {
    downloadJsonFile("chapter_summary_" + stamp + ".json", reports);
    return;
  }
  renderChapterSummary();
  const text = refs.chapterSummaryContent?.textContent || "\u6682\u65e0\u53ef\u7528\u6570\u636e\u3002";
  downloadTextFile("chapter_summary_" + stamp + ".txt", text);
}

function exportEndingReview(asJson) {
  const payload = buildEndingReviewPayload();
  const stamp = Date.now();
  if (asJson) {
    const blob = new Blob([JSON.stringify(payload, null, 2)], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = "ending_review_" + stamp + ".json";
    a.click();
    URL.revokeObjectURL(url);
    return;
  }
  renderEndingReviewContent();
  const txt = refs.endingReviewContent ? refs.endingReviewContent.textContent : JSON.stringify(payload, null, 2);
  const blob = new Blob([txt], { type: "text/plain;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = "ending_review_" + stamp + ".txt";
  a.click();
  URL.revokeObjectURL(url);
}

function evaluateEndingNarrative() {
  const rep = state.factions || { wardens: 0, arcanum: 0, freeguild: 0 };
  const repScore = rep.wardens + rep.arcanum + rep.freeguild;
  const relicScore = state.content.forgedRelics * 2 + state.content.artifactShards;
  const partyScore = (state.party?.roster?.length || 0) * 2;
  const buildScore = (state.talents?.offense || 0) + (state.talents?.guard || 0) + (state.talents?.support || 0);
  const projectScore = (state.projects?.built || 0) * 2;
  const score = repScore + relicScore + partyScore + buildScore + projectScore;
  const decisionBits = getDecisionEndingFragments();

  if (score >= 24) return "\u4f20\u8bf4\u7ed3\u5c40\uff1a\u4f60\u8054\u5408\u8bf8\u9635\u8425\u5c01\u95ed\u88c2\u9699\uff0c\u8fb9\u5883\u8fce\u6765\u65b0\u79e9\u5e8f\u3002" + decisionBits;
  if (score >= 16) return "\u82f1\u96c4\u7ed3\u5c40\uff1a\u4f60\u7a33\u4f4f\u4e86\u524d\u7ebf\uff0c\u4f46\u504f\u8fdc\u5730\u5e26\u4ecd\u6f5c\u4f0f\u52a8\u8361\u3002" + decisionBits;
  return "\u65c5\u8005\u7ed3\u5c40\uff1a\u4f60\u633a\u8fc7\u98ce\u66b4\uff0c\u4e16\u754c\u4ecd\u5728\u7b49\u5f85\u4e0b\u4e00\u4f4d\u5f00\u62d3\u8005\u3002" + decisionBits;
}

function escapeHtml(str) {
  return String(str || "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#39;");
}

function isStoryPinned(id) {
  return (state.storyRuntime?.pinnedStoryCardIds || []).includes(id);
}

function isReplayChapterCollapsed(chapterName) {
  return (state.storyRuntime?.collapsedReplayChapters || []).includes(chapterName);
}

function toggleReplayChapterCollapse(chapterName) {
  ensureGameplayState();
  const arr = state.storyRuntime.collapsedReplayChapters;
  const idx = arr.indexOf(chapterName);
  if (idx >= 0) arr.splice(idx, 1); else arr.push(chapterName);
  renderStoryReplay();
}

function toggleStoryPin(id) {
  ensureGameplayState();
  const arr = state.storyRuntime.pinnedStoryCardIds;
  const idx = arr.indexOf(id);
  if (idx >= 0) arr.splice(idx, 1); else arr.push(id);
  renderStoryReplay();
}

function pushStoryCard(type, title, body) {
  ensureGameplayState();
  const item = {
    id: Date.now() + '_' + Math.floor(Math.random() * 1000),
    day: state.world.day,
    chapter: chapter().name,
    type: type,
    title: title,
    body: body
  };
  state.storyRuntime.storyCards.unshift(item);
  state.storyRuntime.storyCards = state.storyRuntime.storyCards.slice(0, 80);
}

function renderStoryReplay() {
  if (!refs.storyReplayList) return;
  const all = state.storyRuntime?.storyCards || [];
  const filter = refs.selectStoryReplayType?.value || "all";
  const keyword = String(refs.inputStoryReplaySearch?.value || "").trim().toLowerCase();
  const pinnedOnly = Boolean(refs.toggleStoryPinnedOnly?.checked);
  const groupByChapter = refs.toggleStoryGroupByChapter ? Boolean(refs.toggleStoryGroupByChapter.checked) : true;

  const byType = filter === "all" ? all : all.filter((x) => x.type === filter);
  const bySearch = keyword
    ? byType.filter((x) => (x.title || "").toLowerCase().includes(keyword) || (x.body || "").toLowerCase().includes(keyword))
    : byType;
  const byPin = pinnedOnly ? bySearch.filter((x) => isStoryPinned(x.id)) : bySearch;
  const list = byPin.slice().sort((a, b) => Number(isStoryPinned(b.id)) - Number(isStoryPinned(a.id)));

  if (!list.length) {
    refs.storyReplayList.textContent = "\u6ca1\u6709\u5339\u914d\u7684\u5267\u60c5\u8bb0\u5f55\u3002";
    return;
  }

  if (!groupByChapter) {
    refs.storyReplayList.innerHTML = list.map((x, i) => {
      const pinned = isStoryPinned(x.id);
      const pinLabel = pinned ? "\u53d6\u6d88\u7f6e\u9876" : "\u7f6e\u9876";
      return "<div class='rule-item'>"
        + "<div style='display:flex;justify-content:space-between;gap:8px;align-items:center;'>"
        + "<strong>" + (i + 1) + ". [" + escapeHtml(String(x.type || "event").toUpperCase()) + "] [\u7b2c" + escapeHtml(x.day) + "\u5929] [" + escapeHtml(x.chapter) + "] " + escapeHtml(x.title) + "</strong>"
        + "<button class='side-btn compact story-pin' data-story-id='" + escapeHtml(x.id) + "'>" + pinLabel + "</button>"
        + "</div>"
        + "<div style='margin-top:6px;white-space:pre-wrap;'>" + escapeHtml(x.body) + "</div>"
        + "</div>";
    }).join("");
  } else {
    const groups = new Map();
    for (const item of list) {
      const key = String(item.chapter || "\u672a\u77e5\u7ae0\u8282");
      if (!groups.has(key)) groups.set(key, []);
      groups.get(key).push(item);
    }
    const chunks = [];
    let runningIndex = 1;
    for (const [chapterName, cards] of groups.entries()) {
      const collapsed = isReplayChapterCollapsed(chapterName);
      const header = "<div class='rule-item' style='margin-bottom:6px;'>"
        + "<button class='side-btn compact replay-chapter-toggle' data-chapter='" + escapeHtml(chapterName) + "' style='width:100%;text-align:left;'>"
        + (collapsed ? "[+] " : "[-] ") + escapeHtml(chapterName) + "\uff08" + cards.length + "\uff09"
        + "</button>"
        + "</div>";
      chunks.push(header);
      if (!collapsed) {
        for (const x of cards) {
          const pinned = isStoryPinned(x.id);
          const pinLabel = pinned ? "\u53d6\u6d88\u7f6e\u9876" : "\u7f6e\u9876";
          chunks.push("<div class='rule-item' style='margin-left:8px;'>"
            + "<div style='display:flex;justify-content:space-between;gap:8px;align-items:center;'>"
            + "<strong>" + (runningIndex++) + ". [" + escapeHtml(String(x.type || "event").toUpperCase()) + "] [\u7b2c" + escapeHtml(x.day) + "\u5929] " + escapeHtml(x.title) + "</strong>"
            + "<button class='side-btn compact story-pin' data-story-id='" + escapeHtml(x.id) + "'>" + pinLabel + "</button>"
            + "</div>"
            + "<div style='margin-top:6px;white-space:pre-wrap;'>" + escapeHtml(x.body) + "</div>"
            + "</div>");
        }
      }
    }
    refs.storyReplayList.innerHTML = chunks.join("");
  }

  refs.storyReplayList.querySelectorAll(".story-pin").forEach((btn) => {
    btn.addEventListener("click", () => toggleStoryPin(btn.dataset.storyId || ""));
  });
  refs.storyReplayList.querySelectorAll(".replay-chapter-toggle").forEach((btn) => {
    btn.addEventListener("click", () => toggleReplayChapterCollapse(btn.dataset.chapter || ""));
  });
}

function openStoryReplayModal() {
  renderStoryReplay();
  openModal('modalStoryReplay');
}

function buildChapterSummaryReport(c) {
  const rep = state.factions || { wardens: 0, arcanum: 0, freeguild: 0 };
  const decisions = { ...(state.storyRuntime.decisions || {}) };
  const report = {
    chapterId: c.id,
    chapterName: c.name,
    day: state.world.day,
    route: state.story.route,
    level: state.player.level,
    exp: state.player.exp,
    gold: state.player.gold,
    hp: state.player.hp,
    mp: state.player.mp,
    shards: state.content.artifactShards,
    relics: state.content.forgedRelics,
    sideQuestsCompleted: state.storyRuntime.completedSideQuests || 0,
    projectsBuilt: state.projects?.built || 0,
    reputation: { ...rep },
    decisions,
    decisionSummary: buildDecisionSummaryRecords(decisions),
    relationship: npcChapterRelationshipSummary(c.id),
    mission: chapterMissionSummary(c.id)
  };
  return report;
}

function buildTrendLine(label, values) {
  if (!values.length) return label + ": (no data)";
  const min = Math.min(...values);
  const max = Math.max(...values);
  const span = Math.max(1, max - min);
  const bars = values.map((v) => {
    const t = Math.round(((v - min) / span) * 7);
    return " .:-=+*#@"[t] || ".";
  }).join("");
  return label + ": " + bars + "  [" + values.join(",") + "]";
}

function formatDelta(n) {
  if (!Number.isFinite(n) || n === 0) return "0";
  return (n > 0 ? "+" : "") + String(n);
}

function renderChapterPairCompare(reports) {
  if (!refs.compareChapterResult) return;
  if (!reports.length) {
    refs.compareChapterResult.textContent = "\u6682\u65e0\u53ef\u5bf9\u6bd4\u7ae0\u8282\u3002";
    return;
  }

  if (refs.compareChapterA && refs.compareChapterB) {
    const opts = reports.map((r, idx) => "<option value='" + idx + "'>" + (idx + 1) + " - " + escapeHtml(r.chapterName) + "\uff08\u7b2c" + escapeHtml(r.day) + "\u5929\uff09</option>").join("");
    refs.compareChapterA.innerHTML = opts;
    refs.compareChapterB.innerHTML = opts;
    if (!refs.compareChapterA.dataset.inited) {
      refs.compareChapterA.value = "0";
      refs.compareChapterB.value = String(Math.max(0, reports.length - 1));
      refs.compareChapterA.dataset.inited = "1";
      refs.compareChapterB.dataset.inited = "1";
    }
  }

  const ia = Math.max(0, Math.min(reports.length - 1, Number(refs.compareChapterA?.value || 0)));
  const ib = Math.max(0, Math.min(reports.length - 1, Number(refs.compareChapterB?.value || (reports.length - 1))));
  const a = reports[ia];
  const b = reports[ib];
  const txt = [
    "=== \u7ae0\u8282\u5bf9\u6bd4 ===",
    "A: \u7b2c" + (ia + 1) + "\u7ae0 " + a.chapterName + "\uff08\u7b2c" + a.day + "\u5929\uff09",
    "B: \u7b2c" + (ib + 1) + "\u7ae0 " + b.chapterName + "\uff08\u7b2c" + b.day + "\u5929\uff09",
    "B-A \u5dee\u503c -> \u7b49\u7ea7 " + formatDelta((b.level || 0) - (a.level || 0)) +
      " | \u91d1\u5e01 " + formatDelta((b.gold || 0) - (a.gold || 0)) +
      " | HP " + formatDelta((b.hp || 0) - (a.hp || 0)) +
      " | MP " + formatDelta((b.mp || 0) - (a.mp || 0)) +
      " | \u788e\u7247 " + formatDelta((b.shards || 0) - (a.shards || 0)) +
      " | \u5723\u9057\u7269 " + formatDelta((b.relics || 0) - (a.relics || 0)),
    "\u58f0\u671b\u5dee\u503c -> \u5b88\u671b\u8005 " + formatDelta((b.reputation?.wardens || 0) - (a.reputation?.wardens || 0)) +
      " | \u5965\u672f\u8bae\u4f1a " + formatDelta((b.reputation?.arcanum || 0) - (a.reputation?.arcanum || 0)) +
      " | \u81ea\u7531\u884c\u4f1a " + formatDelta((b.reputation?.freeguild || 0) - (a.reputation?.freeguild || 0))
  ].join("\n");
  refs.compareChapterResult.textContent = txt;
}

function renderChapterSummary() {
  if (!refs.chapterSummaryContent) return;
  const reports = state.storyRuntime?.chapterReports || [];
  const withCompare = refs.toggleChapterCompare ? Boolean(refs.toggleChapterCompare.checked) : true;
  if (!reports.length) {
    refs.chapterSummaryContent.textContent = "\u5f53\u524d\u8fd8\u6ca1\u6709\u7ae0\u8282\u603b\u7ed3\uff0c\u8bf7\u5148\u63a8\u8fdb\u7ae0\u8282\u3002";
    if (refs.chapterTrendContent) refs.chapterTrendContent.textContent = "\u6682\u65e0\u8d8b\u52bf\u6570\u636e\u3002";
    if (refs.compareChapterResult) refs.compareChapterResult.textContent = "\u6682\u65e0\u53ef\u5bf9\u6bd4\u7ae0\u8282\u3002";
    return;
  }

  renderChapterPairCompare(reports);

  const levels = reports.map((r) => Number(r.level || 0));
  const golds = reports.map((r) => Number(r.gold || 0));
  const wardens = reports.map((r) => Number(r.reputation?.wardens || 0));
  const arcanum = reports.map((r) => Number(r.reputation?.arcanum || 0));
  const freeguild = reports.map((r) => Number(r.reputation?.freeguild || 0));
  if (refs.chapterTrendContent) {
    refs.chapterTrendContent.textContent = [
      "=== \u7ae0\u8282\u8d8b\u52bf ===",
      buildTrendLine("\u7b49\u7ea7", levels),
      buildTrendLine("\u91d1\u5e01", golds),
      buildTrendLine("\u5b88\u671b\u8005\u58f0\u671b", wardens),
      buildTrendLine("\u5965\u672f\u8bae\u4f1a\u58f0\u671b", arcanum),
      buildTrendLine("\u81ea\u7531\u884c\u4f1a\u58f0\u671b", freeguild)
    ].join("\n");
  }

  refs.chapterSummaryContent.textContent = reports.map((r, idx) => {
    const prev = idx > 0 ? reports[idx - 1] : null;
    const cmpLine = (withCompare && prev)
      ? ("\u76f8\u5bf9\u4e0a\u4e00\u7ae0 -> \u7b49\u7ea7 " + formatDelta((r.level || 0) - (prev.level || 0)) +
        " | \u91d1\u5e01 " + formatDelta((r.gold || 0) - (prev.gold || 0)) +
        " | HP " + formatDelta((r.hp || 0) - (prev.hp || 0)) +
        " | MP " + formatDelta((r.mp || 0) - (prev.mp || 0)) +
        " | \u788e\u7247 " + formatDelta((r.shards || 0) - (prev.shards || 0)))
      : "\u76f8\u5bf9\u4e0a\u4e00\u7ae0 -> (n/a)";
    const decisionRows = Array.isArray(r.decisionSummary) && r.decisionSummary.length
      ? r.decisionSummary
      : buildDecisionSummaryRecords(r.decisions || {});
    const decisionText = decisionRows.length
      ? decisionRows.map((x) => x.title + " -> " + x.optionLabel + (x.impact ? (" (" + x.impact + ")") : "")).join("\uff1b")
      : "\u6682\u65e0\u5173\u952e\u51b3\u7b56";
    const rel = r.relationship || { unlocked: 0, total: 0 };
    return "\u7b2c " + (idx + 1) + " \u7ae0: " + r.chapterName + "\n" +
      "\u5929\u6570: \u7b2c " + r.day + " \u5929 | \u8def\u7ebf: " + r.route + " | \u7b49\u7ea7: " + r.level + "\n" +
      "HP/MP: " + r.hp + "/" + r.mp + " | \u91d1\u5e01: " + r.gold + " | EXP: " + r.exp + "\n" +
      "\u788e\u7247/\u5723\u9057\u7269: " + r.shards + "/" + r.relics + " | \u5efa\u8bbe\u9879\u76ee: " + r.projectsBuilt + "\n" +
      "\u58f0\u671b: \u5b88\u671b\u8005 " + r.reputation.wardens + "\uff0c\u5965\u672f\u8bae\u4f1a " + r.reputation.arcanum + "\uff0c\u81ea\u7531\u884c\u4f1a " + r.reputation.freeguild + "\n" +
      "\u5173\u7cfb\u4e8b\u4ef6: " + rel.unlocked + "/" + rel.total + "\n" +
      cmpLine + "\n" +
      "\u5173\u952e\u51b3\u7b56: " + decisionText;
  }).join("\n\n");
}

function openChapterSummaryModal() {
  renderChapterSummary();
  openModal('modalChapterSummary');
}

function formatSignedValue(v) {
  const n = Number(v || 0);
  return (n >= 0 ? "+" : "") + n;
}

function formatRewardInline(reward) {
  if (!reward) return "";
  const out = [];
  if (reward.exp) out.push("EXP " + formatSignedValue(reward.exp));
  if (reward.gold) out.push("\u91d1\u5e01 " + formatSignedValue(reward.gold));
  if (reward.hp) out.push("HP " + formatSignedValue(reward.hp));
  if (reward.mp) out.push("MP " + formatSignedValue(reward.mp));
  if (reward.shards) out.push("\u788e\u7247 " + formatSignedValue(reward.shards));
  if (reward.atk) out.push("ATK " + formatSignedValue(reward.atk));
  if (reward.def) out.push("DEF " + formatSignedValue(reward.def));
  const pr = reward.progress || {};
  if (pr.explore) out.push("\u63a2\u7d22\u8fdb\u5ea6 " + formatSignedValue(pr.explore));
  if (pr.artifact) out.push("\u9057\u7269\u8fdb\u5ea6 " + formatSignedValue(pr.artifact));
  if (pr.boss) out.push("Boss\u8fdb\u5ea6 " + formatSignedValue(pr.boss));
  return out.join("\u3001");
}

function formatRepInline(rep) {
  const out = [];
  if (typeof rep?.wardens === "number") out.push("\u5b88\u671b\u8005\u58f0\u671b " + formatSignedValue(rep.wardens));
  if (typeof rep?.arcanum === "number") out.push("\u5965\u672f\u8bae\u4f1a\u58f0\u671b " + formatSignedValue(rep.arcanum));
  if (typeof rep?.freeguild === "number") out.push("\u81ea\u7531\u884c\u4f1a\u58f0\u671b " + formatSignedValue(rep.freeguild));
  return out.join("\u3001");
}

function findDecisionDefinition(decisionId) {
  for (const chapterId of Object.keys(CHAPTER_DECISIONS)) {
    for (const def of CHAPTER_DECISIONS[chapterId] || []) {
      if (def.id === decisionId) return { chapterId, def };
    }
  }
  return null;
}

function decisionMechanicHint(decisionId, optionId) {
  if (decisionId === "camp_supply_route" && optionId === "safe_pass") return "Boss\u6218\u5f71\u54cd\uff1a\u53cd\u4f24\u964d\u4f4e\uff0c\u8f93\u51fa\u5c0f\u5e45\u63d0\u5347";
  if (decisionId === "camp_supply_route" && optionId === "rift_shortcut") return "Boss\u6218\u5f71\u54cd\uff1aBoss\u751f\u547d\u964d\u4f4e\uff0c\u4f46\u53cd\u51fb\u66f4\u5f3a";
  if (decisionId === "forest_shrine" && optionId === "seal_shrine") return "Boss\u6218\u5f71\u54cd\uff1a\u538b\u5236\u8282\u594f\u9501\u5b9a\u5173\u95ed";
  if (decisionId === "forest_shrine" && optionId === "draw_power") return "Boss\u6218\u5f71\u54cd\uff1a\u5076\u6570\u56de\u5408\u83b7\u5f97\u989d\u5916\u7206\u53d1\u4f24\u5bb3";
  if (decisionId === "rift_alliance" && optionId === "wardens_line") return "Boss\u6218\u5f71\u54cd\uff1a\u627f\u4f24\u66f4\u7a33\u5b9a";
  if (decisionId === "rift_alliance" && optionId === "arcanum_ritual") return "Boss\u6218\u5f71\u54cd\uff1a\u5468\u671f\u6027\u7206\u53d1\u4f24\u5bb3\u66f4\u9ad8";
  if (decisionId === "rift_alliance" && optionId === "freeguild_charge") return "Boss\u6218\u5f71\u54cd\uff1a\u57fa\u7840\u8f93\u51fa\u63d0\u5347\uff0c\u4f46\u53cd\u4f24\u7565\u589e";
  return "";
}

function buildDecisionSummaryRecords(decisions = state.storyRuntime?.decisions || {}) {
  const rows = [];
  for (const chapterId of Object.keys(CHAPTER_DECISIONS)) {
    for (const def of CHAPTER_DECISIONS[chapterId] || []) {
      const optionId = decisions?.[def.id];
      if (!optionId) continue;
      const option = (def.options || []).find((x) => x.id === optionId) || null;
      const pieces = [];
      const repText = formatRepInline(option?.rep || {});
      const rewardText = formatRewardInline(option?.reward || {});
      const mechanicText = decisionMechanicHint(def.id, optionId);
      if (repText) pieces.push(repText);
      if (rewardText) pieces.push(rewardText);
      if (mechanicText) pieces.push(mechanicText);
      rows.push({
        chapterId,
        decisionId: def.id,
        title: def.title,
        optionId,
        optionLabel: option?.label || optionId,
        impact: pieces.join("\uFF1B")
      });
    }
  }
  return rows;
}

function buildDecisionImpactDetails(decisionId, optionId) {
  const found = findDecisionDefinition(decisionId);
  if (!found) return "";
  const option = (found.def.options || []).find((x) => x.id === optionId);
  if (!option) return "";
  const pieces = [];
  const repText = formatRepInline(option.rep || {});
  const rewardText = formatRewardInline(option.reward || {});
  const mechanicText = decisionMechanicHint(decisionId, optionId);
  if (repText) pieces.push(repText);
  if (rewardText) pieces.push(rewardText);
  if (mechanicText) pieces.push(mechanicText);
  return pieces.join("\uFF1B");
}

function applyDecisionReward(reward) {
  if (!reward) return;
  if (reward.exp || reward.gold) gain(reward.exp || 0, reward.gold || 0);
  if (reward.hp) state.player.hp = Math.max(1, Math.min(state.player.maxHp, state.player.hp + reward.hp));
  if (reward.mp) state.player.mp = Math.max(0, Math.min(state.player.maxMp, state.player.mp + reward.mp));
  if (reward.shards) state.content.artifactShards += reward.shards;
  if (reward.atk) state.player.attackBonus += reward.atk;
  if (reward.def) state.player.defenseBonus += reward.def;
  const pr = reward.progress || {};
  if (pr.explore) state.story.progress.explore += pr.explore;
  if (pr.artifact) state.story.progress.artifact += pr.artifact;
  if (pr.boss) state.story.progress.boss += pr.boss;
}

function getPendingChapterDecision() {
  ensureGameplayState();
  const id = currentChapterId();
  const list = CHAPTER_DECISIONS[id] || [];
  for (const d of list) {
    if (!state.storyRuntime.decisions[d.id]) return d;
  }
  return null;
}

function getDecisionById(id) {
  const map = state.storyRuntime?.decisions || {};
  return map[id] || "";
}

function getDecisionEndingFragments() {
  const pieces = [];
  const camp = getDecisionById("camp_supply_route");
  const forest = getDecisionById("forest_shrine");
  const rift = getDecisionById("rift_alliance");

  if (camp === "safe_pass") pieces.push("\u4f60\u5b88\u4f4f\u4e86\u5546\u8def\uff0c\u8fb9\u5883\u8865\u7ed9\u4f53\u7cfb\u9010\u6b65\u6062\u590d\u3002");
  if (camp === "rift_shortcut") pieces.push("\u4f60\u5011\u5011\u63d0\u4f9b\u4e86\u9ad8\u98ce\u9669\u5feb\u653b\u65b9\u6848\uff0c\u524d\u7ebf\u66f4\u51cc\u5389\u3002");
  if (forest === "seal_shrine") pieces.push("\u88ab\u5c01\u5370\u7684\u795e\u9f9b\u5e73\u606f\u4e86\u6797\u6d77\u5f02\u52a8\u3002");
  if (forest === "draw_power") pieces.push("\u4f60\u6c72\u53d6\u4e86\u795e\u9f9b\u4e4b\u529b\uff0c\u961f\u4f0d\u6218\u529b\u4e0a\u9650\u88ab\u6253\u5f00\u3002");
  if (rift === "wardens_line") pieces.push("\u76fe\u5899\u6218\u672f\u6210\u4e3a\u8054\u5408\u9a7b\u519b\u65b0\u6807\u51c6\u3002");
  if (rift === "arcanum_ritual") pieces.push("\u5965\u672f\u4eea\u5f0f\u7f51\u7edc\u5e38\u9a7b\u5929\u7a7a\uff0c\u5b88\u62a4\u88c2\u9699\u5730\u5e26\u3002");
  if (rift === "freeguild_charge") pieces.push("\u9707\u51fb\u7a81\u88ad\u88ab\u5199\u5165\u524d\u7ebf\u519b\u56e2\u6559\u8303\u3002");

  if (!pieces.length) return "\u4f60\u7684\u7bc7\u7ae0\u5c1a\u672a\u88ab\u8bf8\u9635\u8425\u5b8c\u6574\u8bb0\u8ff0\u3002";
  return pieces.join("");
}

window.FantasyStoryArchiveModule = Object.freeze({
  buildEndingReviewPayload,
  renderEndingReviewContent,
  openEndingReviewModal,
  downloadTextFile,
  downloadJsonFile,
  exportStoryReplay,
  exportChapterSummary,
  exportEndingReview,
  evaluateEndingNarrative,
  escapeHtml,
  isStoryPinned,
  isReplayChapterCollapsed,
  toggleReplayChapterCollapse,
  toggleStoryPin,
  pushStoryCard,
  renderStoryReplay,
  openStoryReplayModal,
  buildChapterSummaryReport,
  buildTrendLine,
  formatDelta,
  renderChapterPairCompare,
  renderChapterSummary,
  openChapterSummaryModal,
  formatSignedValue,
  formatRewardInline,
  formatRepInline,
  findDecisionDefinition,
  decisionMechanicHint,
  buildDecisionSummaryRecords,
  buildDecisionImpactDetails,
  applyDecisionReward,
  getPendingChapterDecision,
  getDecisionById,
  getDecisionEndingFragments
});
