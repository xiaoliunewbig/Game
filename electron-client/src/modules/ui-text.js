(function initFantasyUiTextModule() {
  function applyChineseStoryContent(deps) {
    const {
      STORY_EVENTS = [],
      CAMPFIRE_SCENES = {},
      CHAPTER_ARCS = {},
      NPC_RELATIONSHIP_EVENTS = {}
    } = deps || {};

    const eventTexts = [
      "\u8fb9\u5883\u54e8\u5854\u5347\u8d77\u6c42\u63f4\u7130\u706b\u3002",
      "\u9057\u8ff9\u8109\u51b2\u5728\u8425\u5730\u9644\u8fd1\u663e\u9732\u51fa\u9690\u85cf\u5e9f\u589f\u3002",
      "\u5546\u961f\u8bf7\u6c42\u62a4\u9001\u7a7f\u8d8a\u88c2\u9699\u901a\u9053\u3002",
      "\u654c\u5bf9\u52bf\u529b\u63d0\u51fa\u77ed\u6682\u7ed3\u76df\u3002",
      "\u66b4\u96e8\u51b2\u5237\u51fa\u4e00\u5904\u53e4\u6218\u573a\u3002"
    ];
    for (let i = 0; i < Math.min(STORY_EVENTS.length, eventTexts.length); i += 1) {
      STORY_EVENTS[i] = eventTexts[i];
    }

    const campfire = {
      camp: [
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u8001\u5175\u4eec\u8bb2\u8d77\u7b2c\u4e00\u9053\u8fb9\u5899\u6ca6\u9677\u7684\u90a3\u4e00\u591c\u3002",
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u4f20\u4ee4\u5175\u5728\u6668\u96fe\u4e2d\u753b\u51fa\u5b89\u5168\u884c\u519b\u7ebf\u3002",
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u5b69\u5b50\u4eec\u5531\u8d77\u5b88\u62a4\u5546\u8def\u7684\u9a91\u58eb\u6b4c\u8c23\u3002"
      ],
      forest: [
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u5b88\u6797\u4eba\u4f4e\u8bed\u6797\u6839\u4f1a\u56de\u5e94\u53e4\u8001\u540d\u5b57\u3002",
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u9057\u7269\u730e\u4eba\u5766\u8a00\u81ea\u5df1\u5bb3\u6015\u795e\u9f9b\u7684\u94c3\u58f0\u3002",
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u4fa6\u67e5\u5175\u6807\u8bb0\u51fa\u8dc3\u8fc7\u5f71\u517d\u7684\u79d8\u5f84\u3002"
      ],
      rift: [
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u5404\u65b9\u5728\u6700\u540e\u88c2\u9699\u524d\u7acb\u4e0b\u5171\u6218\u8a93\u8a00\u3002",
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u5de5\u7a0b\u5e08\u4e89\u8bba\u62a4\u76fe\u6845\u80fd\u5426\u6491\u5230\u5929\u4eae\u3002",
        "\u7bc7\u706b\u6545\u4e8b\uff1a\u751f\u8fd8\u8005\u63cf\u8ff0\u65ad\u5c42\u9644\u8fd1\u626d\u66f2\u7684\u65f6\u95f4\u56de\u58f0\u3002"
      ]
    };

    for (const key of Object.keys(campfire)) {
      if (!Array.isArray(CAMPFIRE_SCENES[key])) continue;
      for (let i = 0; i < Math.min(CAMPFIRE_SCENES[key].length, campfire[key].length); i += 1) {
        CAMPFIRE_SCENES[key][i] = campfire[key][i];
      }
    }

    const arcText = {
      camp_watchtower: ["\u54e8\u5854\u544a\u6025", "\u5728\u88ad\u51fb\u8005\u7a81\u7834\u524d\u589e\u63f4\u8fb9\u9632\u54e8\u5854\u3002"],
      camp_supply_chain: ["\u8865\u7ed9\u7ebf\u91cd\u5efa", "\u62a4\u9001\u8f66\u961f\u5e76\u8ffd\u56de\u6c11\u5175\u4e22\u5931\u7684\u519b\u8d44\u3002"],
      camp_outer_wall: ["\u5916\u5899\u575a\u5b88", "\u9ec4\u660f\u9632\u7ebf\u627f\u538b\uff0c\u51fb\u9000\u534f\u540c\u653b\u52bf\u3002"],
      forest_echo_roots: ["\u56de\u54cd\u6839\u8109", "\u6cbf\u7740\u79fb\u52a8\u6811\u7ebf\u8ffd\u8e2a\u9057\u7269\u8109\u51b2\u3002"],
      forest_shrine_guard: ["\u795e\u9f9b\u5b88\u536b", "\u5b88\u62a4\u4eea\u5f0f\u5708\uff0c\u4e3a\u7b26\u6587\u7a33\u5b9a\u4e89\u53d6\u65f6\u95f4\u3002"],
      forest_night_hunt: ["\u591c\u730e", "\u6e05\u526a\u5f71\u517d\uff0c\u9632\u6b62\u5176\u8fd1\u903c\u53cb\u519b\u8425\u5730\u3002"],
      rift_anchor_alpha: ["\u951a\u70b9\u963f\u5c14\u6cd5", "\u5728\u9ad8\u538b\u4e0b\u542f\u52a8\u7b2c\u4e00\u9053\u88c2\u9699\u951a\u70b9\u3002"],
      rift_anchor_beta: ["\u951a\u70b9\u8d1d\u5854", "\u62a4\u9001\u5de5\u7a0b\u961f\u5e76\u5efa\u7acb\u7b2c\u4e8c\u9053\u963b\u9694\u7ebf\u3002"],
      rift_final_muster: ["\u7ec8\u5c40\u96c6\u7ed3", "\u96c6\u7ed3\u5168\u90e8\u76df\u53cb\uff0c\u8fce\u63a5\u51b3\u5b9a\u6027\u4e00\u6218\u3002"]
    };

    for (const key of Object.keys(CHAPTER_ARCS)) {
      for (const node of CHAPTER_ARCS[key]) {
        const text = arcText[node.id];
        if (!text) continue;
        node.title = text[0];
        node.desc = text[1];
      }
    }

    const relText = {
      watch_oath: ["\u54e8\u8a93", "\u57c3\u4e3d\u62c9\u5c06\u54e8\u5854\u5de1\u903b\u5bc6\u94a5\u4ea4\u7ed9\u4f60\u3002"],
      dawn_signal: ["\u6668\u4fe1\u53f7", "\u4f60\u4e0e\u54e8\u5854\u5b8c\u6210\u6c11\u5175\u5e94\u6025\u54cd\u5e94\u534f\u8bae\u3002"],
      caravan_contract: ["\u8f66\u961f\u5951\u7ea6", "\u6d1b\u683c\u5728\u5730\u56fe\u4e0a\u65b0\u589e\u4e86\u4e00\u6761\u9690\u852d\u8865\u7ed9\u8def\u7ebf\u3002"],
      market_reopen: ["\u66ae\u8272\u96c6\u5e02", "\u7070\u9053\u96c6\u5e02\u91cd\u542f\uff0c\u524d\u7ebf\u7269\u8d44\u4ef7\u683c\u56de\u843d\u3002"],
      medic_station: ["\u6d41\u52a8\u533b\u7ad9", "\u5b89\u5a1c\u4e3a\u4f60\u7684\u5c0f\u961f\u5f00\u542f\u5e94\u6025\u6551\u6cbb\u7ad9\u3002"],
      triage_protocol: ["\u5206\u8bca\u89c4\u7a0b", "\u4f60\u4eec\u638c\u63e1\u4e86\u6218\u5730\u5206\u8bca\u8282\u594f\uff0c\u961f\u4f0d\u97e7\u6027\u63d0\u5347\u3002"],
      rune_archive: ["\u7b26\u6587\u6863\u6848", "\u7ef4\u5c14\u5206\u4eab\u4e86\u9057\u7269\u8109\u51b2\u7b14\u8bb0\uff0c\u641c\u7d22\u6548\u7387\u63d0\u9ad8\u3002"],
      resonance_map: ["\u5171\u632f\u56fe\u8c31", "\u4f60\u5b8c\u6210\u4e86\u68ee\u57df\u5171\u632f\u6807\u6ce8\uff0c\u5bfb\u7269\u8def\u7ebf\u66f4\u660e\u786e\u3002"],
      hunter_mark: ["\u730e\u624b\u5370\u8bb0", "\u51ef\u6069\u4e3a\u4f60\u6807\u51fa\u4e86\u5f71\u517d\u9ad8\u5371\u884c\u8fdb\u7ebf\u3002"],
      night_pursuit: ["\u591c\u884c\u8ffd\u730e", "\u4f60\u7684\u5c0f\u961f\u638c\u63e1\u4e86\u591c\u6218\u8ffd\u51fb\u8282\u594f\u3002"],
      warding_light: ["\u795e\u9f9b\u706f\u706b", "\u7f07\u59c6\u5411\u4f60\u5f00\u653e\u4e86\u5723\u57df\u7ed3\u754c\u8282\u70b9\u3002"],
      forest_aegis: ["\u68ee\u57df\u5e87\u62a4", "\u68ee\u6797\u7ed3\u754c\u80fd\u591f\u5316\u89e3\u4e00\u6b21\u5173\u952e\u51b2\u51fb\u3002"],
      anchor_tuning: ["\u951a\u70b9\u8c03\u8bd5", "\u6258\u6bd4\u5141\u8bb8\u4f60\u53c2\u4e0e\u524d\u7ebf\u951a\u70b9\u7cbe\u5ea6\u8c03\u8bd5\u3002"],
      stability_loop: ["\u7a33\u5b9a\u56de\u8def", "\u4f60\u638c\u63e1\u4e86\u88c2\u9699\u538b\u5236\u5e94\u6025\u7a0b\u5e8f\u3002"],
      shield_drill: ["\u76fe\u5899\u8f6e\u6362", "\u6258\u4f26\u6559\u4f1a\u4f60\u9663\u5f62\u8f6e\u6362\u6280\u672f\uff0c\u9632\u7ebf\u66f4\u7a33\u3002"],
      last_watch: ["\u6700\u540e\u591c\u5b88", "\u7b2c\u4e09\u9632\u7ebf\u5b8c\u6210\u4e86\u540c\u6b65\u591c\u5b88\u6392\u73ed\u3002"],
      storm_window: ["\u98ce\u66b4\u7a97\u53e3", "\u96f7\u514b\u5411\u4f60\u900f\u9732\u4e86\u88c2\u96fe\u77ed\u7a97\u65f6\u673a\u3002"],
      breaker_charge: ["\u7834\u9635\u51b2\u51fb", "\u5148\u950b\u5c0f\u961f\u4e0e\u4f60\u5b8c\u6210\u4e86\u540c\u6b65\u51b2\u51fb\u8282\u594f\u3002"]
    };

    for (const key of Object.keys(NPC_RELATIONSHIP_EVENTS)) {
      const list = NPC_RELATIONSHIP_EVENTS[key];
      if (!Array.isArray(list)) continue;
      for (const evt of list) {
        const text = relText[evt.id];
        if (!text) continue;
        evt.title = text[0];
        evt.body = text[1];
      }
    }
  }

  function applyChineseUiText(deps) {
    const { refs = {} } = deps || {};
    const setText = (id, text) => {
      const el = document.getElementById(id);
      if (el) el.textContent = text;
    };

    document.title = "\u5e7b\u60f3\u4f20\u5947 - Electron \u5ba2\u6237\u7aef";
    const brandTitle = document.querySelector('.brand h1');
    if (brandTitle) brandTitle.textContent = "\u5e7b\u60f3\u4f20\u5947";
    const brandSub = document.querySelector('.brand span');
    if (brandSub) brandSub.textContent = 'Electron \u5ba2\u6237\u7aef';

    const heroTitle = document.querySelector('#viewMainMenu h2');
    if (heroTitle) heroTitle.textContent = "\u4e3b\u83dc\u5355";
    const heroSub = document.querySelector('#viewMainMenu .subtitle');
    if (heroSub) heroSub.textContent = "\u5728\u88c2\u53d8\u8fb9\u5883\u4e66\u5199\u4f60\u7684\u4f20\u5947\u3002";
    setText('btnNewGame', "\u65b0\u6e38\u620f");
    setText('btnLoadGame', "\u8bfb\u53d6\u5b58\u6863");
    setText('btnSettings', "\u8bbe\u7f6e");
    setText('btnExit', "\u9000\u51fa");
    const worldHint = document.querySelector('.world-header span:last-child');
    if (worldHint) worldHint.textContent = "\u79fb\u52a8: WASD / \u65b9\u5411\u952e | \u4ea4\u4e92: E | \u53cc\u51fb\u4f20\u9001";
    setText('btnStoryDecision', "\u5267\u60c5\u6289\u62e9");
    setText('btnExplore', "\u63a2\u7d22\u533a\u57df");
    setText('btnRest', "\u8425\u5730\u4f11\u606f");
    setText('btnUsePotion', "\u4f7f\u7528\u836f\u6c34");
    setText('btnAdvanceChapter', "\u63a8\u8fdb\u7ae0\u8282");
    setText('btnCommitChoice', "\u786e\u8ba4\u8def\u7ebf");
    setText('btnInventory', "\u80cc\u5305 (I)");
    setText('btnSkillTree', "\u5929\u8d4b (K)");
    setText('btnSave', "\u4fdd\u5b58 (F5)");
    setText('btnBackMenu', "\u8fd4\u56de\u4e3b\u83dc\u5355 (Esc)");
    setText('btnViewEndingReview', "\u7ed3\u5c40\u590d\u76d8");
    setText('btnCastSkill', "\u91ca\u653e\u6280\u80fd");
    setText('btnCompanionSkill', "\u540c\u4f34\u6280\u80fd");
    setText('btnQuest', "\u59d4\u6258\u9762\u677f");
    setText('btnRefreshSideQuests', "\u5237\u65b0\u652f\u7ebf\u4efb\u52a1");
    setText('btnCampfireStory', "\u8425\u706b\u5267\u60c5");
    setText('btnBuildProject', "\u5efa\u8bbe\u5730\u533a\u9879\u76ee");
    setText('btnRefreshShop', "\u5237\u65b0\u5546\u5e97");
    setText('btnBuyShopItem', "\u8d2d\u4e70\u7269\u8d44");
    setText('btnForgeRelic', "\u953b\u9020\u5723\u7269");
    setText('btnStartBoss', "\u5f00\u59cb Boss \u6218");
    setText('btnStrikeBoss', "\u653b\u51fb Boss");
    setText('btnClearBossTimeline', "\u6e05\u7a7a\u65f6\u95f4\u7ebf");
    setText('btnRecruitCompanion', "\u62db\u52df\u540c\u4f34");
    setText('btnTalentInfo', "\u5929\u8d4b\u8bf4\u660e");
    setText('btnTalentAuto', "\u81ea\u52a8\u5206\u914d");
    setText('btnStoryReplay', "\u5267\u60c5\u56de\u987e");
    setText('btnChapterSummary', "\u7ae0\u8282\u603b\u7ed3");
    setText('btnStoryGuide', "\u5267\u60c5\u5f15\u5bfc");
    setText('npcPanelTitle', 'NPC \u4ea4\u4e92');
    setText('btnOpenNpcDialog', "\u9644\u8fd1 NPC");
    setText('npcDialogTitle', 'NPC \u5bf9\u8bdd');
    setText('btnNpcTalk', "\u4ea4\u8c08");
    setText('btnNpcQuest', "\u59d4\u6258");
    setText('btnNpcTrade', "\u4ea4\u6613");
    setText('btnNpcTrain', "\u8bad\u7ec3");
    setText('btnNpcGift', "\u8d60\u793c");
    const npcCloseBtn = document.querySelector('[data-close="modalNpcDialog"]');
    if (npcCloseBtn) npcCloseBtn.textContent = "\u5173\u95ed";
    setText('storyDecisionTitle', "\u5267\u60c5\u6289\u62e9");
    setText('btnExportStoryReplayTxt', "\u5bfc\u51fa TXT");
    setText('btnExportStoryReplayJson', "\u5bfc\u51fa JSON");
    setText('btnExportChapterSummaryTxt', "\u5bfc\u51fa TXT");
    setText('btnExportChapterSummaryJson', "\u5bfc\u51fa JSON");

    const replayModal = document.getElementById('modalStoryReplay');
    if (replayModal) {
      const title = replayModal.querySelector('h3');
      if (title) title.textContent = "\u5267\u60c5\u56de\u987e";
    }
    const summaryModal = document.getElementById('modalChapterSummary');
    if (summaryModal) {
      const title = summaryModal.querySelector('h3');
      if (title) title.textContent = "\u7ae0\u8282\u603b\u7ed3";
    }
    if (refs.inputStoryReplaySearch) refs.inputStoryReplaySearch.placeholder = "\u641c\u7d22\u5267\u60c5\u5173\u952e\u8bcd";

    const pinnedLabel = refs.toggleStoryPinnedOnly?.closest('label');
    if (pinnedLabel) {
      const txt = pinnedLabel.childNodes[pinnedLabel.childNodes.length - 1];
      if (txt) txt.textContent = "\u4ec5\u770b\u7f6e\u9876";
    }
    const groupLabel = refs.toggleStoryGroupByChapter?.closest('label');
    if (groupLabel) {
      const txt = groupLabel.childNodes[groupLabel.childNodes.length - 1];
      if (txt) txt.textContent = "\u6309\u7ae0\u8282\u5206\u7ec4";
    }

    if (refs.selectStoryReplayType) {
      refs.selectStoryReplayType.innerHTML = "<option value='all'>\u5168\u90e8\u7c7b\u578b</option><option value='arc'>\u5267\u60c5\u7ebf</option><option value='decision'>\u6289\u62e9</option><option value='event'>\u4e8b\u4ef6</option><option value='chapter'>\u7ae0\u8282</option>";
    }

    const storyTitle = refs.storySummary?.closest('.panel-block')?.querySelector('h3');
    if (storyTitle) storyTitle.textContent = "\u5267\u60c5\u4e0e\u4efb\u52a1";
    const partyTitle = refs.partyList?.closest('.panel-block')?.querySelector('h3');
    if (partyTitle) partyTitle.textContent = "\u961f\u4f0d\u4e0e\u6210\u957f";
    const skillTitle = refs.skillSummary?.closest('.panel-block')?.querySelector('h3');
    if (skillTitle) skillTitle.textContent = "\u6280\u80fd\u4e0e\u6784\u7b51";
    const bossTitle = refs.bossStatus?.closest('.panel-block')?.querySelector('h3');
    if (bossTitle) bossTitle.textContent = 'Boss \u6218\u6597';
  }

  window.FantasyUiTextModule = Object.freeze({
    applyChineseStoryContent,
    applyChineseUiText
  });
})();
