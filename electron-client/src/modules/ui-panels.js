(function initFantasyUiPanelsModule() {
  const {
    CHAPTERS,
    CHAPTER_ARCS
  } = window.FantasyGameData || {};

  function createUiPanelsModule(deps) {
    const {
      state,
      refs,
      chapter,
      currentChapterId,
      ensureGameplayState,
      getStoryProgressStatus,
      getChapterArcState,
      getCurrentArcNode,
      chapterMissionSummary,
      missionDone,
      missionMetricValue,
      chapterSupportSummary,
      supportPrimaryBlockerText,
      activeCompanion,
      regionPoiSummary,
      renderSupportTracker,
      renderPartyAndSkills,
      renderNpcPanel
    } = deps || {};

    if (!state || !refs) {
      throw new Error("createUiPanelsModule requires state and refs.");
    }

    function renderStoryAndQuests() {
      ensureGameplayState?.();
      const current = chapter();
      const progress = Number(state.story.progress[current.key] || 0);
      const sideDone = Number(state.storyRuntime.completedSideQuests || 0);
      const beats = Number(state.storyRuntime.beats || 0);
      const arcNodes = CHAPTER_ARCS[current.id] || [];
      const arcState = getChapterArcState?.(current.id) || { index: 0, completed: false };
      const arcNext = getCurrentArcNode?.(current.id);
      const status = getStoryProgressStatus?.(current) || {};

      const keyLabelMap = {
        explore: "探索",
        artifact: "遗物",
        boss: "首领"
      };
      const routeLabel = {
        order: "秩序",
        balanced: "均衡",
        chaos: "混沌"
      };
      const mainKey = keyLabelMap[current.key] || current.key;
      const arcLine = arcNodes.length
        ? ("剧情线进度: " + Math.min(arcState.index, arcNodes.length) + "/" + arcNodes.length + (arcNext ? (" | 下一节点: " + arcNext.title) : " | 已完成"))
        : "剧情线进度: 无";
      const missionLine = status.mission?.total
        ? ("章节任务: " + status.mission.done + "/" + status.mission.total + (status.mission.next ? (" | 下一项: " + status.mission.next.title) : " | 已全部完成"))
        : "章节任务: 无";
      const poi = regionPoiSummary?.(current.id) || { total: 0, done: 0, nearest: null };
      const poiLine = poi.total
        ? ("区域地标: " + poi.done + "/" + poi.total + (poi.nearest ? (" | 附近: " + (poi.nearest.def?.name || poi.nearest.id)) : ""))
        : "区域地标: 无";

      const guideLines = ["剧情指引: " + (status.nextAction || "继续推进当前章节")];
      if (status.pendingDecision) {
        guideLines.push("可选抉择: 点击【剧情抉择】处理《" + status.pendingDecision.title + "》");
      }

      if (refs.storySummary) {
        refs.storySummary.textContent =
          "章节: " + current.name + "\n"
          + "主目标: " + mainKey + " " + Math.min(progress, current.target) + "/" + current.target + "\n"
          + arcLine + "\n"
          + missionLine + "\n"
          + poiLine + "\n"
          + "路线: " + (routeLabel[state.story.route] || routeLabel.balanced) + " | 剧情节拍: " + beats + " | 支线完成: " + sideDone + "\n"
          + guideLines.join("\n");
      }

      if (refs.questList) {
        refs.questList.innerHTML = "";

        const guideDiv = document.createElement("div");
        guideDiv.className = "quest-item" + (status.canAdvance ? " done" : "");
        guideDiv.textContent = status.canAdvance
          ? "本章条件已满足，可点击“推进章节”继续。"
          : "下一步: " + (status.nextAction || "继续探索");
        refs.questList.appendChild(guideDiv);

        CHAPTERS.forEach((entry, index) => {
          const item = document.createElement("div");
          item.className = "quest-item" + (index < state.story.chapterIndex ? " done" : "");
          const currentProgress = Math.min(state.story.progress[entry.key] || 0, entry.target);
          const label = keyLabelMap[entry.key] || entry.key;
          item.textContent = (index === state.story.chapterIndex ? "[进行中] " : "") + entry.name + " | " + label + ": " + currentProgress + "/" + entry.target;
          refs.questList.appendChild(item);
        });

        if (arcNodes.length) {
          const arcDiv = document.createElement("div");
          arcDiv.className = "quest-item" + (arcState.completed ? " done" : "");
          arcDiv.textContent = "主线节点 " + Math.min(arcState.index, arcNodes.length) + "/" + arcNodes.length + (arcNext ? (" -> " + arcNext.title) : "（已完结）");
          refs.questList.appendChild(arcDiv);
        }

        if (poi.total) {
          const poiRow = document.createElement("div");
          poiRow.className = "quest-item" + (poi.done >= poi.total ? " done" : "");
          const nextPoiName = poi.nearest ? (poi.nearest.def?.name || poi.nearest.id) : "无";
          poiRow.textContent = "[区域地标] " + poi.done + "/" + poi.total + " | 下一个目标: " + nextPoiName + " | 可双击地图快速到达";
          refs.questList.appendChild(poiRow);
        }

        if (status.mission?.total) {
          status.mission.defs.forEach((missionDef) => {
            const done = missionDone?.(missionDef, current.id);
            const currentMetric = Math.min(missionMetricValue?.(missionDef.metric, current.id) || 0, Number(missionDef.target || 0));
            const row = document.createElement("div");
            row.className = "quest-item" + (done ? " done" : "");
            row.textContent = "[章节任务] " + missionDef.title + " | " + currentMetric + "/" + missionDef.target;
            refs.questList.appendChild(row);
          });
        }

        if (status.pendingDecision) {
          const pendingDiv = document.createElement("div");
          pendingDiv.className = "quest-item";
          pendingDiv.textContent = "可选抉择: " + status.pendingDecision.title + "（在“剧情抉择”中处理）";
          refs.questList.appendChild(pendingDiv);
        }
      }

      if (refs.storyChoiceSelect) {
        refs.storyChoiceSelect.innerHTML = "<option value='order'>秩序路线</option><option value='balanced'>均衡路线</option><option value='chaos'>混沌路线</option>";
        refs.storyChoiceSelect.value = state.story.choice || state.story.route;
      }

      const endingText = state.storyRuntime.ending ? ("\n结局: " + state.storyRuntime.ending) : "";
      if (refs.choiceSummary) {
        refs.choiceSummary.textContent =
          "当前路线: " + (routeLabel[state.story.choice] || "未选择")
          + "\n说明: 秩序 = 更稳的防御，均衡 = 属性平均，混沌 = 更高爆发"
          + "\n推进建议: " + (status.nextAction || "继续探索")
          + endingText;
      }
    }

    function renderPanels() {
      const repData = state.factions || { wardens: 0, arcanum: 0, freeguild: 0 };
      const sideDone = Number(state.storyRuntime?.completedSideQuests || 0);
      const storyStatus = getStoryProgressStatus?.(chapter()) || { nextAction: "继续探索当前章节" };
      const mission = chapterMissionSummary?.(currentChapterId()) || { total: 0, done: 0, next: null };
      const activeMate = activeCompanion?.();
      const poi = regionPoiSummary?.(currentChapterId()) || { total: 0, done: 0, nearest: null };
      const support = chapterSupportSummary?.(currentChapterId()) || { total: 0, completed: 0, current: null, currentStatus: null };
      const supportLine = !support.total
        ? "协作行动: 当前章节暂无内容"
        : support.current
          ? ("协作行动: " + support.current.title + (support.currentStatus?.ok ? " | 条件已满足" : (" | 仍需: " + supportPrimaryBlockerText?.(support.current, support.currentStatus, currentChapterId()))))
          : ("协作行动: 已完成 " + support.completed + "/" + support.total);
      const missionLine = mission.total
        ? ("章节任务: " + mission.done + "/" + mission.total + (mission.next ? (" | 下一项: " + mission.next.title) : ""))
        : "章节任务: 当前章节暂无特殊任务";
      const companionLine = activeMate
        ? ("同行同伴: " + activeMate.name + " Lv." + activeMate.level + " | " + activeMate.skillName + " | 羁绊 " + activeMate.bond)
        : "同行同伴: 还没有招募到同伴";

      if (refs.factionReputation) {
        refs.factionReputation.textContent = "阵营声望: 守望者 " + repData.wardens + " | 奥术议会 " + repData.arcanum + " | 自由行会 " + repData.freeguild;
      }
      if (refs.projectSummary) {
        refs.projectSummary.textContent =
          "区域: [" + state.world.regionX + ", " + state.world.regionY + "]\n"
          + "资源: 碎片 " + state.content.artifactShards + " | 遗物 " + state.content.forgedRelics + " | 支线完成 " + sideDone + "\n"
          + "区域地标: " + poi.done + "/" + poi.total + (poi.nearest ? (" | 附近: " + (poi.nearest.def?.name || poi.nearest.id)) : "") + "\n"
          + "剧情建议: " + storyStatus.nextAction + "\n"
          + missionLine + "\n"
          + supportLine + "\n"
          + companionLine;
      }
      if (refs.forgeSummary) {
        refs.forgeSummary.textContent =
          "消耗 3 个遗物碎片进行锻造\n"
          + "当前碎片: " + state.content.artifactShards + "\n"
          + "可用天赋点: " + (state.talents.points || 0);
      }

      if (refs.sideQuestList) {
        refs.sideQuestList.innerHTML = "";
        if (!state.sideQuests.length) {
          const empty = document.createElement("div");
          empty.className = "quest-item";
          empty.textContent = "当前还没有支线任务，可以点击“刷新委托”生成新的任务。";
          refs.sideQuestList.appendChild(empty);
        } else {
          state.sideQuests.forEach((quest) => {
            const item = document.createElement("div");
            item.className = "quest-item" + (quest.completed ? " done" : "");
            item.textContent = quest.title + " | " + Math.min(quest.progress, quest.target) + "/" + quest.target + " | 奖励 EXP " + quest.rewardExp + " | 金币 +" + quest.rewardGold;
            refs.sideQuestList.appendChild(item);
          });
        }
      }

      if (refs.journalLog) {
        refs.journalLog.textContent = (state.journal || []).slice(0, 8).join("\n") || "日志将记录剧情、协作、支线与 NPC 互动。";
      }

      renderSupportTracker?.();
      renderPartyAndSkills?.();
      renderNpcPanel?.();
    }

    return Object.freeze({
      renderPanels,
      renderStoryAndQuests
    });
  }

  window.FantasyUiPanelsModule = Object.freeze({
    createUiPanelsModule
  });
})();