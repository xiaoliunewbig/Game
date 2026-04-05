(function () {
const CHAPTERS = [
  { id: "camp", name: "\u6668\u98ce\u8425\u5730", key: "explore", target: 3, exp: 30, gold: 60 },
  { id: "forest", name: "\u4f4e\u8bed\u6797\u5730", key: "artifact", target: 3, exp: 60, gold: 120 },
  { id: "rift", name: "\u88c2\u9699\u524d\u7ebf", key: "boss", target: 1, exp: 120, gold: 220 }
];

const JOBS = {
  warrior: { label: "\u6218\u58eb", hp: 130, mp: 50, atk: 18, def: 12 },
  mage: { label: "\u6cd5\u5e08", hp: 95, mp: 90, atk: 22, def: 7 },
  archer: { label: "\u5f13\u7bad\u624b", hp: 110, mp: 70, atk: 20, def: 9 }
};

const SKILL_LIBRARY = [
  { id: "quick_strike", name: "\u8fc5\u5203\u65a9", mp: 10, kind: "attack", bonus: 5 },
  { id: "guard_break", name: "\u7834\u7532\u51fb", mp: 14, kind: "attack", bonus: 9 },
  { id: "aether_burst", name: "\u4ee5\u592a\u7206\u53d1", mp: 18, kind: "attack", bonus: 14 },
  { id: "vital_surge", name: "\u6d3b\u529b\u6d8c\u52a8", mp: 16, kind: "heal" },
  { id: "frost_barrier", name: "\u971c\u6676\u5c4f\u969c", mp: 15, kind: "shield" },
  { id: "thunder_spear", name: "\u96f7\u9706\u4e4b\u67aa", mp: 22, kind: "attack", bonus: 20 },
  { id: "shadow_step", name: "\u5f71\u6b65\u7a81\u88ad", mp: 12, kind: "attack", bonus: 7 },
  { id: "blazing_arc", name: "\u708e\u5f27\u65a9", mp: 20, kind: "attack", bonus: 16 },
  { id: "starfall", name: "\u661f\u5760\u672f", mp: 26, kind: "attack", bonus: 24 },
  { id: "wind_grace", name: "\u98ce\u606f\u62a4\u4f51", mp: 14, kind: "support" },
  { id: "chrono_anchor", name: "\u65f6\u5f8b\u951a\u5b9a", mp: 20, kind: "support" },
  { id: "lunar_rain", name: "\u6708\u8680\u6d41\u661f", mp: 28, kind: "attack", bonus: 28 }
];

const COMPANION_ARCHETYPES = [
  {
    key: "guardian",
    name: "钢盾·阿澜",
    role: "守护",
    atk: 1,
    def: 3,
    support: 1,
    specialty: "护阵",
    field: "前线掩护",
    tactic: "在队伍前排吸收伤害并维持阵线",
    skillName: "壁垒誓约",
    passive: "生命越低，减伤越高"
  },
  {
    key: "ranger",
    name: "岚羽",
    role: "游猎",
    atk: 3,
    def: 1,
    support: 1,
    specialty: "侦察",
    field: "林地追踪",
    tactic: "优先锁定残血目标并提供远程火力",
    skillName: "穿风箭雨",
    passive: "首次出手提高暴击"
  },
  {
    key: "mystic",
    name: "星见",
    role: "秘术",
    atk: 1,
    def: 1,
    support: 3,
    specialty: "共鸣",
    field: "遗迹支援",
    tactic: "维持队伍法力与护盾，擅长续航",
    skillName: "月泉祷歌",
    passive: "施放支援技能后回复少量法力"
  },
  {
    key: "vanguard",
    name: "烬锋",
    role: "先锋",
    atk: 2,
    def: 2,
    support: 1,
    specialty: "突进",
    field: "裂隙突袭",
    tactic: "快速切入高危目标，对 Boss 保持压制",
    skillName: "破阵轰袭",
    passive: "对首领目标造成额外伤害"
  }
];

const STORY_EVENTS = [
  "\u8fb9\u5883\u54e8\u5854\u5347\u8d77\u6c42\u63f4\u7130\u706b\u3002",
  "\u9057\u8ff9\u8109\u51b2\u5728\u8425\u5730\u9644\u8fd1\u663e\u9732\u51fa\u9690\u85cf\u5e9f\u589f\u3002",
  "\u5546\u961f\u8bf7\u6c42\u62a4\u9001\u7a7f\u8d8a\u88c2\u9699\u901a\u9053\u3002",
  "\u654c\u5bf9\u52bf\u529b\u63d0\u51fa\u77ed\u6682\u7ed3\u76df\u3002",
  "\u66b4\u96e8\u51b2\u5237\u51fa\u4e00\u5904\u53e4\u6218\u573a\u3002"
];

const CHAPTER_STORYLINES = {
  camp: [
    "\u6668\u98ce\u6c11\u5175\u8bf7\u6c42\u62a4\u9001\u8865\u7ed9\u8f66\u961f\u3002",
    "\u5931\u8e2a\u4fa6\u67e5\u5175\u7559\u4e0b\u4e86\u7834\u635f\u5730\u56fe\u788e\u7247\u3002",
    "\u6751\u6c11\u6b63\u4e89\u8bba\u662f\u5426\u64a4\u79bb\u8fb9\u5883\u3002"
  ],
  forest: [
    "\u4f4e\u8bed\u6797\u6d77\u5bf9\u9057\u7269\u8109\u51b2\u4ea7\u751f\u4e86\u56de\u54cd\u3002",
    "\u9690\u853d\u795e\u9f9b\u63d0\u4f9b\u5f3a\u5927\u529b\u91cf\uff0c\u4f46\u4ee3\u4ef7\u96be\u6d4b\u3002",
    "\u6e38\u4fa0\u62a5\u544a\u5f71\u517d\u6b63\u7a7f\u8d8a\u53e4\u8001\u8fb9\u754c\u3002"
  ],
  rift: [
    "\u88c2\u9699\u98ce\u66b4\u4f7f\u524d\u7ebf\u51fa\u73b0\u65f6\u95f4\u626d\u66f2\u3002",
    "\u5404\u65b9\u52bf\u529b\u5728\u524d\u7ebf\u7ec4\u5efa\u8054\u5408\u6218\u8bae\u3002",
    "\u53e4\u8001\u5b88\u62a4\u8005\u5728\u65ad\u5c42\u4e0b\u65b9\u82cf\u9192\u3002"
  ]
};

const CHAPTER_ARCS = {
  camp: [
    {
      id: "camp_watchtower",
      title: "Watchtower Distress",
      desc: "Reinforce the frontier watchtower before raiders break through.",
      reward: { exp: 24, gold: 30, rep: { wardens: 1 }, progress: { explore: 1 } }
    },
    {
      id: "camp_supply_chain",
      title: "Supply Chain Recovery",
      desc: "Secure caravan routes and recover lost crates for the militia.",
      reward: { exp: 26, gold: 34, rep: { wardens: 1, freeguild: 1 }, progress: { explore: 1 } }
    },
    {
      id: "camp_outer_wall",
      title: "Outer Wall Stand",
      desc: "Hold the wall at dusk and repel a coordinated assault.",
      reward: { exp: 32, gold: 40, rep: { wardens: 1 }, progress: { explore: 1 } }
    }
  ],
  forest: [
    {
      id: "forest_echo_roots",
      title: "Echoing Roots",
      desc: "Track relic pulses through moving tree lines.",
      reward: { exp: 34, gold: 36, shards: 1, rep: { arcanum: 1 }, progress: { artifact: 1 } }
    },
    {
      id: "forest_shrine_guard",
      title: "Shrine Ward",
      desc: "Defend ritual circles while wardens stabilize shrine runes.",
      reward: { exp: 36, gold: 38, rep: { wardens: 1, arcanum: 1 }, progress: { artifact: 1 } }
    },
    {
      id: "forest_night_hunt",
      title: "Night Hunt",
      desc: "Purge shadow beasts before they reach allied camps.",
      reward: { exp: 40, gold: 48, rep: { freeguild: 1 }, progress: { artifact: 1 } }
    }
  ],
  rift: [
    {
      id: "rift_anchor_alpha",
      title: "Anchor Alpha",
      desc: "Power the first breach anchor under heavy pressure.",
      reward: { exp: 46, gold: 56, rep: { arcanum: 1 }, progress: { boss: 1 } }
    },
    {
      id: "rift_anchor_beta",
      title: "Anchor Beta",
      desc: "Escort engineers and establish a second containment line.",
      reward: { exp: 48, gold: 60, rep: { wardens: 1, freeguild: 1 }, progress: { boss: 1 } }
    },
    {
      id: "rift_final_muster",
      title: "Final Muster",
      desc: "Gather every ally and prepare for the decisive boss clash.",
      reward: { exp: 56, gold: 72, rep: { wardens: 1, arcanum: 1, freeguild: 1 }, progress: { boss: 1 } }
    }
  ]
};

const BOSS_PROFILES = {
  camp: { name: "\u94a2\u94c1\u5148\u950b", hpBase: 120, hpPerLevel: 30, reflect: 0.08, cadence: 3 },
  forest: { name: "\u6839\u987b\u5148\u77e5", hpBase: 150, hpPerLevel: 34, reflect: 0.1, cadence: 2 },
  rift: { name: "\u88c2\u9699\u4e3b\u5bb0", hpBase: 185, hpPerLevel: 38, reflect: 0.12, cadence: 2 }
};

const CHAPTER_DECISIONS = {
  camp: [
    {
      id: "camp_supply_route",
      title: "\u8865\u7ed9\u7ebf\u5371\u673a",
      desc: "\u8f66\u961f\u53ef\u4ee5\u8d70\u5b89\u5168\u5c71\u9053\uff0c\u4e5f\u53ef\u4ee5\u5192\u9669\u88c2\u9699\u8fd1\u8def\u3002",
      options: [
        { id: "safe_pass", label: "\u62a4\u9001\u5b89\u5168\u5c71\u9053", scene: "\u57c3\u4e3d\u62c9\u961f\u957f\uff1a\u7a33\u4f4f\u9635\u5f62\uff0c\u4e00\u8f86\u90fd\u4e0d\u80fd\u4e22\u3002", require: { min_reputation: { wardens: 0 } }, rep: { wardens: 1 }, reward: { exp: 14, gold: 12, progress: { explore: 1 } } },
        { id: "rift_shortcut", label: "\u5192\u9669\u88c2\u9699\u8fd1\u8def", scene: "\u5362\u514b\u8d70\u79c1\u5ba2\uff1a\u8f7b\u88c5\u7a81\u8fdb\uff0c\u4ece\u88c2\u96fe\u4e2d\u51b2\u51fa\u53bb\u3002", require: { min_companions: 1 }, rep: { freeguild: 1 }, reward: { exp: 10, gold: 32, hp: -6, progress: { explore: 1 } } }
      ]
    }
  ],
  forest: [
    {
      id: "forest_shrine",
      title: "\u4f4e\u8bed\u795e\u9f9b",
      desc: "\u795e\u9f9b\u6b63\u91ca\u653e\u4e0d\u7a33\u5b9a\u7684\u9057\u7269\u529b\u91cf\u3002",
      options: [
        { id: "seal_shrine", label: "\u5c01\u5370\u795e\u9f9b", scene: "\u795e\u5b98\uff1a\u5148\u675f\u7f1a\u6839\u8109\uff0c\u518d\u8c08\u529b\u91cf\u3002", require: { min_reputation: { wardens: 1 } }, rep: { wardens: 1 }, reward: { exp: 16, shards: 1, progress: { artifact: 1 } } },
        { id: "draw_power", label: "\u6c72\u53d6\u795e\u9f9b\u4e4b\u529b", scene: "\u6848\u5377\u5b98\uff1a\u5c06\u8109\u51b2\u5f15\u5165\u4f60\u7684\u6838\u5fc3\u3002", require: { min_shards: 1 }, rep: { arcanum: 1 }, reward: { exp: 22, mp: 12, hp: -5, progress: { artifact: 1 } } }
      ]
    }
  ],
  rift: [
    {
      id: "rift_alliance",
      title: "\u7ec8\u5c40\u8054\u76df\u8868\u51b3",
      desc: "\u4e09\u65b9\u9635\u8425\u8981\u6c42\u6267\u884c\u4e0d\u540c\u7684\u4f5c\u6218\u6559\u4e49\u3002",
      options: [
        { id: "wardens_line", label: "\u575a\u5b88\u76fe\u5899\u9635\u7ebf", scene: "\u6258\u4f26\u5143\u5e05\uff1a\u6211\u4eec\u5728\u6b64\u575a\u5b88\uff0c\u7edd\u4e0d\u540e\u9000\u3002", require: { min_reputation: { wardens: 2 } }, rep: { wardens: 2 }, reward: { def: 1, exp: 20 } },
        { id: "arcanum_ritual", label: "\u5f3a\u5316\u5965\u672f\u4eea\u5f0f\u5708", scene: "\u7ef4\u5c14\u5927\u6cd5\u5e08\uff1a\u7ed9\u4eea\u5f0f\u5708\u4f9b\u80fd\uff0c\u5c01\u95ed\u4e3b\u88c2\u53e3\u3002", require: { min_reputation: { arcanum: 2 } }, rep: { arcanum: 2 }, reward: { mp: 14, exp: 20 } },
        { id: "freeguild_charge", label: "\u53d1\u8d77\u9707\u51fb\u7a81\u88ad", scene: "\u5362\u514b\u8239\u957f\uff1a\u5148\u624b\u51fa\u51fb\uff0c\u4e00\u6ce2\u6253\u7a7f\uff01", require: { min_companions: 2 }, rep: { freeguild: 2 }, reward: { atk: 1, gold: 45, hp: -8, exp: 20 } }
      ]
    }
  ]
};

const CAMPFIRE_SCENES = {
  camp: [
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u8001\u5175\u4eec\u8bb2\u8d77\u7b2c\u4e00\u9053\u8fb9\u5899\u6ca6\u9677\u7684\u90a3\u4e00\u591c\u3002",
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u4f20\u4ee4\u5175\u5728\u6668\u96fe\u4e2d\u753b\u51fa\u5b89\u5168\u884c\u519b\u7ebf\u3002",
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u5b69\u5b50\u4eec\u5531\u8d77\u5b88\u62a4\u5546\u8def\u7684\u9a91\u58eb\u6b4c\u8c23\u3002"
  ],
  forest: [
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u5b88\u6797\u4eba\u4f4e\u8bed\u6797\u6839\u4f1a\u56de\u5e94\u53e4\u65e7\u540d\u5b57\u3002",
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u9057\u7269\u730e\u4eba\u5766\u8a00\u81ea\u5df1\u5bb3\u6015\u795e\u9f9b\u94c3\u58f0\u3002",
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u4fa6\u67e5\u5175\u6807\u8bb0\u51fa\u8dc3\u8fc7\u5f71\u517d\u7684\u79d8\u5f84\u3002"
  ],
  rift: [
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u5404\u65b9\u5728\u6700\u540e\u88c2\u9699\u524d\u7acb\u4e0b\u5171\u6218\u8a93\u8a00\u3002",
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u5de5\u7a0b\u5e08\u4e89\u8bba\u62a4\u76fe\u6845\u80fd\u5426\u6491\u5230\u5929\u4eae\u3002",
    "\u7bc7\u706b\u6545\u4e8b\uff1a\u751f\u8fd8\u8005\u63cf\u8ff0\u65ad\u5c42\u9644\u8fd1\u626d\u66f2\u7684\u65f6\u95f4\u56de\u58f0\u3002"
  ]
};

const NPC_PROFILES = {
  camp: [
    { key: "npc_watch_lieutenant", name: "\u57c3\u4e3d\u62c9", title: "\u54e8\u5854\u526f\u961f\u957f", faction: "wardens", questKind: "explore", tradeType: "supply", dialogue: "\u8fb9\u9632\u7ebf\u8def\u9700\u8981\u66f4\u7a33\u7684\u4fa6\u5bdf\u4e0e\u8855\u63a5\u3002" },
    { key: "npc_route_merchant", name: "\u6d1b\u683c", title: "\u5546\u8def\u7ecf\u7eaa", faction: "freeguild", questKind: "combat", tradeType: "potion", dialogue: "\u53ea\u8981\u8f66\u961f\u80fd\u8fc7\u53bb\uff0c\u8d27\u4ef7\u5c31\u80fd\u56de\u843d\u3002" },
    { key: "npc_field_medic", name: "\u5b89\u5a1c", title: "\u6218\u5730\u533b\u5e08", faction: "wardens", questKind: "artifact", tradeType: "herb", dialogue: "\u6218\u7ebf\u4e0a\u7684\u6bcf\u4e00\u74f6\u836f\u90fd\u80fd\u6539\u5199\u7ed3\u5c40\u3002" }
  ],
  forest: [
    { key: "npc_rune_scholar", name: "\u7ef4\u5c14", title: "\u7b26\u6587\u7814\u7a76\u5458", faction: "arcanum", questKind: "artifact", tradeType: "shard", dialogue: "\u9057\u7269\u8109\u51b2\u7684\u89c4\u5f8b\u9700\u8981\u66f4\u591a\u89c2\u6d4b\u70b9\u3002" },
    { key: "npc_beast_hunter", name: "\u51ef\u6069", title: "\u5f71\u517d\u730e\u4eba", faction: "freeguild", questKind: "combat", tradeType: "weapon", dialogue: "\u4eca\u591c\u8ffd\u8e2a\u7684\u75d5\u8ff9\u6307\u5411\u66f4\u6df1\u7684\u6797\u6d77\u3002" },
    { key: "npc_shrine_keeper", name: "\u7f07\u59c6", title: "\u795e\u9f9b\u5b88\u62a4\u8005", faction: "wardens", questKind: "explore", tradeType: "ward", dialogue: "\u5723\u57df\u7684\u57fa\u77f3\u9700\u8981\u4f60\u53bb\u91cd\u65b0\u70b9\u4eae\u3002" }
  ],
  rift: [
    { key: "npc_front_engineer", name: "\u6258\u6bd4", title: "\u88c2\u9699\u5de5\u7a0b\u5e08", faction: "arcanum", questKind: "artifact", tradeType: "tool", dialogue: "\u951a\u70b9\u7a33\u5b9a\u5ea6\u8fd8\u5dee\u4e24\u4e2a\u6821\u51c6\u73af\u8282\u3002" },
    { key: "npc_shield_captain", name: "\u6258\u4f26", title: "\u76fe\u5899\u6307\u6325\u5b98", faction: "wardens", questKind: "explore", tradeType: "armor", dialogue: "\u7b2c\u4e09\u9632\u7ebf\u521a\u5b8c\u6210\u90e8\u7f72\uff0c\u73b0\u5728\u9700\u8981\u5de1\u68c0\u3002" },
    { key: "npc_breaker_scout", name: "\u96f7\u514b", title: "\u88c2\u96fe\u4fa6\u67e5\u5175", faction: "freeguild", questKind: "combat", tradeType: "burst", dialogue: "\u98ce\u66b4\u7a7a\u7a97\u53ea\u6709\u51e0\u5206\u949f\uff0c\u6211\u4eec\u5f97\u8d76\u7d27\u3002" }
  ]
};

const NPC_QUEST_TEMPLATES = {
  explore: { title: "\u3010\u59d4\u6258\u3011\u8fb9\u754c\u5de1\u68c0", target: 2, rewardGold: 90, rewardExp: 70 },
  combat: { title: "\u3010\u59d4\u6258\u3011\u526a\u9664\u5a01\u80c1", target: 2, rewardGold: 110, rewardExp: 84 },
  artifact: { title: "\u3010\u59d4\u6258\u3011\u9057\u7269\u91c7\u96c6", target: 2, rewardGold: 100, rewardExp: 78 }
};

const NPC_STORY_CHAINS = {
  camp: {
    npc_watch_lieutenant: ["\u6821\u51c6\u54e8\u5854\u4fe1\u53f7", "\u5de1\u68c0\u98ce\u66b4\u7f3a\u53e3", "\u7ec4\u7ec7\u6c11\u5175\u8f6e\u6362"],
    npc_route_merchant: ["\u786e\u8ba4\u5546\u8def\u7ad9\u70b9", "\u62a4\u9001\u8f66\u961f\u7a7f\u8d8a\u7070\u9053", "\u91cd\u5f00\u66ae\u8272\u96c6\u5e02"],
    npc_field_medic: ["\u6e05\u70b9\u533b\u7597\u8865\u7ed9", "\u8f6c\u79fb\u4f24\u5458\u5230\u5b89\u5168\u533a", "\u5efa\u7acb\u6d41\u52a8\u6551\u62a4\u7ad9"]
  },
  forest: {
    npc_rune_scholar: ["\u89e3\u6790\u9057\u7269\u6ce2\u5f62", "\u7a33\u5b9a\u6811\u6d77\u7b26\u9635", "\u5b8c\u6210\u795e\u9f9b\u8bb0\u5f55\u4e66"],
    npc_beast_hunter: ["\u8ffd\u8e2a\u5f71\u517d\u9ad8\u5371\u533a", "\u56f4\u526a\u591c\u884c\u5de2\u7a74", "\u5f00\u8f9f\u6797\u95f4\u5b89\u5168\u7ebf"],
    npc_shrine_keeper: ["\u70b9\u4eae\u5931\u843d\u795e\u9f9b", "\u62a4\u9001\u796d\u793c\u7b26\u77f3", "\u5b8c\u6210\u68ee\u57df\u7ed3\u754c\u4fee\u7f2e"]
  },
  rift: {
    npc_front_engineer: ["\u6821\u51c6\u951a\u70b9\u7535\u5bb9", "\u66f4\u6362\u7834\u635f\u62a4\u76fe\u74e6\u7247", "\u542f\u52a8\u5e94\u6025\u538b\u5236\u73af"],
    npc_shield_captain: ["\u68c0\u67e5\u7b2c\u4e09\u9632\u7ebf", "\u8c03\u6574\u76fe\u5899\u9635\u5f62", "\u7ec4\u7ec7\u88c2\u9699\u591c\u5b88\u961f"],
    npc_breaker_scout: ["\u6807\u8bb0\u98ce\u66b4\u7a7a\u7a97", "\u5f15\u5bfc\u5148\u950b\u7a81\u51fb\u7ebf", "\u5b8c\u6210\u88c2\u96fe\u5f3a\u88ad\u6848"]
  }
};

const NPC_RELATIONSHIP_EVENTS = {
  npc_watch_lieutenant: [
    { id: "watch_oath", trust: 4, title: "Watch Oath", body: "Elira grants patrol keys for the Dawnwatch tower.", reward: { exp: 20, gold: 24, rep: { wardens: 1 } } },
    { id: "dawn_signal", trust: 7, title: "Dawn Signal", body: "You establish a militia response protocol with the watchtower.", reward: { def: 1, progress: { explore: 1 } } }
  ],
  npc_route_merchant: [
    { id: "caravan_contract", trust: 4, title: "Caravan Pact", body: "Logg adds a hidden supply path to your route map.", reward: { exp: 18, gold: 38, rep: { freeguild: 1 } } },
    { id: "market_reopen", trust: 7, title: "Dusk Market", body: "The grayway market reopens and lowers front-line prices.", reward: { gold: 48, progress: { explore: 1 } } }
  ],
  npc_field_medic: [
    { id: "medic_station", trust: 4, title: "Field Station", body: "Anna opens an emergency medical station for your squad.", reward: { hp: 12, exp: 18 } },
    { id: "triage_protocol", trust: 7, title: "Triage Protocol", body: "Your team learns battlefield triage for stronger sustain.", reward: { def: 1, rep: { wardens: 1 } } }
  ],
  npc_rune_scholar: [
    { id: "rune_archive", trust: 4, title: "Rune Archive", body: "Ver shares relic pulse notes and improves search accuracy.", reward: { shards: 1, exp: 22, rep: { arcanum: 1 } } },
    { id: "resonance_map", trust: 7, title: "Resonance Map", body: "You map forest resonance and track relics faster.", reward: { progress: { artifact: 1 }, mp: 10 } }
  ],
  npc_beast_hunter: [
    { id: "hunter_mark", trust: 4, title: "Hunter Mark", body: "Kane marks high-risk shadow-beast routes for you.", reward: { exp: 20, gold: 26, progress: { artifact: 1 } } },
    { id: "night_pursuit", trust: 7, title: "Night Pursuit", body: "Your team learns nocturnal pursuit rhythm.", reward: { atk: 1, rep: { freeguild: 1 } } }
  ],
  npc_shrine_keeper: [
    { id: "warding_light", trust: 4, title: "Shrine Light", body: "Tim lets you access sanctuary ward nodes.", reward: { shards: 1, hp: 10, rep: { wardens: 1 } } },
    { id: "forest_aegis", trust: 7, title: "Forest Aegis", body: "The barrier now dampens one critical impact window.", reward: { def: 1, progress: { artifact: 1 } } }
  ],
  npc_front_engineer: [
    { id: "anchor_tuning", trust: 4, title: "Anchor Tuning", body: "Toby lets you assist with front-line anchor tuning.", reward: { exp: 24, shards: 1, rep: { arcanum: 1 } } },
    { id: "stability_loop", trust: 7, title: "Stability Loop", body: "You master emergency stability loops for breach control.", reward: { progress: { boss: 1 }, mp: 12 } }
  ],
  npc_shield_captain: [
    { id: "shield_drill", trust: 4, title: "Shield Drill", body: "Toren teaches formation rotation for better defense.", reward: { exp: 22, def: 1, rep: { wardens: 1 } } },
    { id: "last_watch", trust: 7, title: "Last Watch", body: "The third line enters synchronized night watch.", reward: { progress: { boss: 1 }, gold: 30 } }
  ],
  npc_breaker_scout: [
    { id: "storm_window", trust: 4, title: "Storm Window", body: "Rek shares short-lived breach storm windows.", reward: { exp: 22, gold: 28, rep: { freeguild: 1 } } },
    { id: "breaker_charge", trust: 7, title: "Breaker Charge", body: "Vanguard squads synchronize strike timing with you.", reward: { atk: 1, progress: { boss: 1 } } }
  ]
};

const NPC_TRADE_PROFILES = {
  supply: { mode: "supply", costBase: 38, costFloor: 20, label: "\u524d\u7ebf\u8865\u7ed9" },
  herb: { mode: "supply", costBase: 34, costFloor: 18, label: "\u533b\u7597\u7bb1" },
  potion: { mode: "supply", costBase: 36, costFloor: 20, label: "\u7d27\u6025\u836f\u5242" },
  shard: { mode: "shard", costBase: 42, costFloor: 22, label: "\u7b26\u77f3\u788e\u7247" },
  tool: { mode: "shard", costBase: 40, costFloor: 22, label: "\u5de5\u7a0b\u7ec4\u4ef6" },
  weapon: { mode: "weapon", costBase: 44, costFloor: 24, label: "\u6b66\u88c5\u96f6\u4ef6" },
  burst: { mode: "weapon", costBase: 46, costFloor: 24, label: "\u7206\u88c2\u836f\u82af" },
  ward: { mode: "armor", costBase: 40, costFloor: 22, label: "\u7ed3\u754c\u62a4\u7b26" },
  armor: { mode: "armor", costBase: 42, costFloor: 22, label: "\u91cd\u88c5\u8865\u7247" }
};
const CHAPTER_MISSIONS = {
  camp: [
    { id: "camp_recon", title: "\u524d\u6cbf\u4fa6\u5bdf", metric: "explore_steps", target: 2, actionId: "btnExplore", actionLabel: "\u7ee7\u7eed\u63a2\u7d22", reward: { exp: 22, gold: 34 } },
    { id: "camp_liaison", title: "\u4e0e\u9635\u8425\u8054\u7edc", metric: "npc_contacts", target: 2, actionId: "btnOpenNpcDialog", actionLabel: "\u524d\u5f80 NPC \u4ea4\u4e92", reward: { exp: 18, gold: 26, rep: { wardens: 1 } } },
    { id: "camp_contract", title: "\u5b8c\u6210\u59d4\u6258\u4ea4\u4ed8", metric: "npc_quests_completed", target: 1, actionId: "btnOpenNpcDialog", actionLabel: "\u5b8c\u6210 NPC \u59d4\u6258", reward: { exp: 26, gold: 40, progress: { explore: 1 } } }
  ],
  forest: [
    { id: "forest_relic", title: "\u91c7\u96c6\u9057\u7269\u788e\u7247", metric: "artifact_found", target: 2, actionId: "btnExplore", actionLabel: "\u63a2\u7d22\u5bfb\u627e\u788e\u7247", reward: { exp: 24, gold: 30, shards: 1 } },
    { id: "forest_story", title: "\u63a8\u8fdb\u5267\u60c5\u8282\u70b9", metric: "story_events", target: 1, actionId: "btnTriggerStory", actionLabel: "\u89e6\u53d1\u5267\u60c5\u4e8b\u4ef6", reward: { exp: 20, gold: 22, rep: { arcanum: 1 } } },
    { id: "forest_board", title: "\u652f\u7ebf\u59d4\u6258\u4e24\u8fde", metric: "side_quests_completed", target: 2, actionId: "btnRefreshSideQuests", actionLabel: "\u5237\u65b0\u5e76\u5b8c\u6210\u652f\u7ebf", reward: { exp: 28, gold: 44, progress: { artifact: 1 } } }
  ],
  rift: [
    { id: "rift_coord", title: "\u88c2\u9699\u524d\u7ebf\u534f\u540c", metric: "npc_contacts", target: 2, actionId: "btnOpenNpcDialog", actionLabel: "\u4e0e\u524d\u7ebf NPC \u8054\u7cfb", reward: { exp: 26, gold: 36, rep: { freeguild: 1 } } },
    { id: "rift_pressure", title: "\u538b\u5236\u9996\u9886\u8282\u594f", metric: "boss_hits", target: 3, actionId: "btnStrikeBoss", actionLabel: "\u6301\u7eed\u653b\u51fb Boss", reward: { exp: 34, gold: 48 } },
    { id: "rift_finale", title: "\u7ec8\u5c40\u51b3\u6218", metric: "boss_kills", target: 1, actionId: "btnStartBoss", actionLabel: "\u5f00\u542f\u5e76\u5b8c\u6210 Boss \u6218", reward: { exp: 42, gold: 66, progress: { boss: 1 } } }
  ]
};


const CHAPTER_SUPPORT_OPERATIONS = {
  camp: [
    {
      id: "camp_signal_chain",
      title: "重建烽火信链",
      desc: "把晨风营地的烽火塔、旗语台和侦察哨重新连成一条预警网络，确保前线能第一时间传回情报。",
      actionLabel: "重启信链",
      costGold: 70,
      costShards: 0,
      require: { chainCompleted: 1, relationUnlocked: 1, poiVisited: 1 },
      reward: { exp: 28, gold: 20, hp: 12, rep: { wardens: 1 }, progress: { explore: 1 }, mission: { story_events: 1 } },
      journal: "晨风营地重新点亮了烽火信链，边境的预警速度明显提升。"
    },
    {
      id: "camp_field_hospital",
      title: "搭建野战医站",
      desc: "协调医师、药剂师和守望者，在营地外围搭建简易医站，为后续远征提供稳定补给。",
      actionLabel: "搭建医站",
      costGold: 95,
      costShards: 0,
      require: { missionDone: 2, relationUnlocked: 2, requireCompanion: true },
      reward: { exp: 34, gold: 26, hp: 18, def: 1, rep: { wardens: 1, arcanum: 1 }, progress: { explore: 1 } },
      journal: "野战医站开始运作，伤员与巡逻队终于拥有稳定的补给点。"
    }
  ],
  forest: [
    {
      id: "forest_resonance_tower",
      title: "修复共鸣高塔",
      desc: "修复林地遗迹中的共鸣高塔，让散落的遗物信号重新汇聚，帮助队伍锁定更深层的线索。",
      actionLabel: "修复高塔",
      costGold: 95,
      costShards: 1,
      require: { missionDone: 1, relationUnlocked: 1, poiVisited: 1 },
      reward: { exp: 36, gold: 24, mp: 14, shards: 1, rep: { arcanum: 1 }, progress: { artifact: 1 }, mission: { story_events: 1 } },
      journal: "低语林地的共鸣高塔再次运转，遗物脉冲被稳定地记录下来。"
    },
    {
      id: "forest_moonwell",
      title: "净化月井",
      desc: "清理月井周围的裂隙污染，让这处古老泉源重新为远征队提供恢复与庇护。",
      actionLabel: "净化月井",
      costGold: 120,
      costShards: 1,
      require: { chainCompleted: 2, relationUnlocked: 2, sideQuests: 2, requireCompanion: true },
      reward: { exp: 44, gold: 28, mp: 18, def: 1, rep: { arcanum: 1, freeguild: 1 }, progress: { artifact: 1 } },
      journal: "月井重新恢复澄明，林地中的回复气息再次流动。"
    }
  ],
  rift: [
    {
      id: "rift_bastion_grid",
      title: "铺设棱堡阵列",
      desc: "在裂隙前线建立多层棱堡与掩体阵列，让守军能在风暴与怪潮中维持阵地。",
      actionLabel: "铺设阵列",
      costGold: 125,
      costShards: 1,
      require: { missionDone: 1, relationUnlocked: 2, requireCompanion: true },
      reward: { exp: 48, gold: 32, atk: 1, def: 1, rep: { wardens: 1, freeguild: 1 }, progress: { boss: 1 } },
      journal: "棱堡阵列完成部署，前线终于有了能够稳住局势的防御骨架。"
    },
    {
      id: "rift_starbridge",
      title: "架设星桥",
      desc: "利用遗物能量与前线工事，在断裂地带架设一条临时星桥，为最终推进争取通路。",
      actionLabel: "架设星桥",
      costGold: 150,
      costShards: 2,
      require: { chainCompleted: 2, missionDone: 2, poiVisited: 2, requireCompanion: true },
      reward: { exp: 58, gold: 44, atk: 1, mp: 18, rep: { wardens: 1, arcanum: 1, freeguild: 1 }, progress: { boss: 1 }, mission: { boss_hits: 1 } },
      journal: "星桥横跨裂隙前线，联合部队终于获得了冲向终局的道路。"
    }
  ]
};

const REGION_POI_LIBRARY = {
  camp: [
    {
      id: "watchtower",
      type: "watch",
      name: "\u524d\u7ebf\u54e8\u5854",
      desc: "\u54e8\u5854\u9700\u8981\u4f60\u91cd\u7f6e\u4f20\u706b\u4fe1\u53f7\u3002",
      metric: "explore_steps",
      reward: { exp: 16, gold: 18, progress: { explore: 1 } }
    },
    {
      id: "supply_cache",
      type: "cache",
      name: "\u6218\u5730\u8865\u7ed9\u70b9",
      desc: "\u627e\u5230\u88ab\u9690\u85cf\u7684\u8865\u7ed9\u7bb1\uff0c\u52a0\u56fa\u524d\u7ebf\u7eed\u822a\u3002",
      metric: "npc_contacts",
      reward: { gold: 24, hp: 8, progress: { explore: 1 } }
    },
    {
      id: "militia_camp",
      type: "camp",
      name: "\u6c11\u5175\u4e34\u65f6\u8425",
      desc: "\u7ec4\u7ec7\u6c11\u5175\u8f6e\u5b88\uff0c\u4e3a\u8425\u5730\u4e89\u53d6\u8c03\u5ea6\u65f6\u95f4\u3002",
      metric: "story_events",
      reward: { exp: 20, rep: { wardens: 1 }, progress: { explore: 1 } }
    }
  ],
  forest: [
    {
      id: "rune_pool",
      type: "rune",
      name: "\u7b26\u6587\u6c60",
      desc: "\u5f15\u5bfc\u9057\u7269\u80fd\u91cf\u8fdb\u5165\u7a33\u5b9a\u56de\u8def\u3002",
      metric: "artifact_found",
      reward: { shards: 1, exp: 18, progress: { artifact: 1 } }
    },
    {
      id: "shrine_path",
      type: "shrine",
      name: "\u795e\u9f9b\u5c0f\u5f84",
      desc: "\u6e05\u7406\u5f71\u517d\u8db3\u8ff9\uff0c\u4fdd\u8bc1\u4eea\u5f0f\u961f\u53ef\u4ee5\u901a\u884c\u3002",
      metric: "combat_wins",
      reward: { exp: 20, gold: 20, rep: { freeguild: 1 }, progress: { artifact: 1 } }
    },
    {
      id: "echo_obelisk",
      type: "obelisk",
      name: "\u56de\u54cd\u65b9\u5c16\u7891",
      desc: "\u6821\u51c6\u65b9\u5c16\u7891\u9891\u7387\uff0c\u83b7\u5f97\u65b0\u7684\u4e3b\u7ebf\u7ebf\u7d22\u3002",
      metric: "story_events",
      reward: { exp: 22, mp: 10, rep: { arcanum: 1 }, progress: { artifact: 1 } }
    }
  ],
  rift: [
    {
      id: "anchor_node",
      type: "anchor",
      name: "\u951a\u70b9\u63a7\u5236\u53f0",
      desc: "\u8c03\u6574\u951a\u70b9\u53c2\u6570\uff0c\u538b\u5236\u88c2\u9699\u98ce\u66b4\u5f3a\u5ea6\u3002",
      metric: "boss_hits",
      reward: { exp: 24, gold: 26, progress: { boss: 1 } }
    },
    {
      id: "signal_beacon",
      type: "beacon",
      name: "\u8054\u519b\u4fe1\u6807",
      desc: "\u70b9\u4eae\u4fe1\u6807\uff0c\u5404\u9635\u8425\u589e\u63f4\u6210\u529f\u7387\u63d0\u5347\u3002",
      metric: "npc_contacts",
      reward: { exp: 22, rep: { wardens: 1, arcanum: 1, freeguild: 1 }, progress: { boss: 1 } }
    },
    {
      id: "storm_gate",
      type: "gate",
      name: "\u88c2\u96fe\u95f8\u53e3",
      desc: "\u5f00\u542f\u77ed\u6682\u7a33\u5b9a\u95f8\u53e3\uff0c\u4e3a Boss \u6218\u521b\u9020\u7a97\u53e3\u671f\u3002",
      metric: "boss_hits",
      reward: { exp: 26, gold: 30, atk: 1, progress: { boss: 1 } }
    }
  ]
};

window.FantasyGameData = Object.freeze({
  CHAPTERS,
  JOBS,
  SKILL_LIBRARY,
  COMPANION_ARCHETYPES,
  STORY_EVENTS,
  CHAPTER_STORYLINES,
  CHAPTER_ARCS,
  BOSS_PROFILES,
  CHAPTER_DECISIONS,
  CAMPFIRE_SCENES,
  NPC_PROFILES,
  NPC_QUEST_TEMPLATES,
  NPC_STORY_CHAINS,
  NPC_RELATIONSHIP_EVENTS,
  NPC_TRADE_PROFILES,
  CHAPTER_MISSIONS,
  CHAPTER_SUPPORT_OPERATIONS,
  REGION_POI_LIBRARY
});
})();
