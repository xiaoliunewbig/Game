// Combat, skill, and boss helpers extracted from renderer.js.

const gameServices = window.FantasyGameServices;

if (!gameServices) {
  throw new Error("FantasyGameServices failed to load.");
}


function renderBoss() {
  if (!refs.bossStatus) return;
  if (!state.combat.active) {
    refs.bossStatus.textContent = "\u5f53\u524d\u6ca1\u6709 Boss \u6218\u6597\u3002";
    refs.worldMap?.classList.remove("boss-pulse", "boss-shake", "boss-rift");
    refs.bossTurnInfo && (refs.bossTurnInfo.textContent = "\u56de\u5408: -");
    refs.bossTimeline && (refs.bossTimeline.textContent = "\u65f6\u95f4\u7ebf\u4e3a\u7a7a\u3002");
    return;
  }
  const phase = state.combat.hp <= state.combat.maxHp * 0.35 ? "\u72c2\u66b4" : "\u7a33\u5b9a";
  refs.bossStatus.textContent = state.combat.bossName + "\nHP: " + state.combat.hp + "/" + state.combat.maxHp + "\n\u9636\u6bb5: " + phase;
  refs.bossTurnInfo && (refs.bossTurnInfo.textContent = "\u56de\u5408: " + (state.combat.turn || (state.combat.timeline.length + 1)));
  refs.bossTimeline && (refs.bossTimeline.textContent = state.combat.timeline.slice(-12).join("\n"));
}

function renderPartyAndSkills() {
  ensureGameplayState();
  ensureCompanionRosterState();
  if (refs.partyList) {
    refs.partyList.innerHTML = "";
    if (!state.party.roster.length) {
      refs.partyList.textContent = "\u8fd8\u6ca1\u6709\u540c\u4f34\uff0c\u53ef\u82b1\u8d39 80 \u91d1\u5e01\u62db\u52df\u3002";
    } else {
      state.party.roster.forEach((m) => {
        const row = document.createElement("div");
        row.className = "party-item" + (m.id === state.party.activeId ? " active" : "");

        const name = document.createElement("div");
        name.className = "party-item-name";
        name.textContent = m.name + " / " + m.role;

        const meta = document.createElement("div");
        meta.className = "party-item-meta";
        meta.textContent = "Lv." + m.level + " | \u7f81\u7eca " + m.bond + " | \u6218\u529b " + companionPower(m);

        const note = document.createElement("div");
        note.className = "party-item-note";
        note.textContent = m.skillName + " | " + m.specialty + " | " + m.passive;

        row.appendChild(name);
        row.appendChild(meta);
        row.appendChild(note);
        refs.partyList.appendChild(row);
      });
    }
  }

  if (refs.companionSelect) {
    const selected = state.party.activeId || refs.companionSelect.value || "";
    refs.companionSelect.innerHTML = state.party.roster.length
      ? state.party.roster.map((m) => "<option value='" + m.id + "'>" + m.name + " - Lv." + m.level + " - " + m.skillName + "</option>").join("")
      : "<option value=''>\u6682\u65e0\u540c\u4f34</option>";
    refs.companionSelect.value = state.party.roster.some((m) => m.id === selected) ? selected : (state.party.roster[0]?.id || "");
  }

  if (refs.skillSelect) {
    const selectedSkill = refs.skillSelect.value || state.skillLoadout?.[0] || "quick_strike";
    refs.skillSelect.innerHTML = SKILL_LIBRARY.map((x) => "<option value='" + x.id + "'>" + x.name + " (" + x.mp + " MP)</option>").join("");
    if (SKILL_LIBRARY.some((x) => x.id === selectedSkill)) refs.skillSelect.value = selectedSkill;
  }
}

function applyTalent(type) {
  ensureGameplayState();
  if ((state.talents.points || 0) <= 0) return showToast("\u5929\u8d4b\u70b9\u4e0d\u8db3");
  state.talents.points -= 1;
  if (type === "offense") {
    state.talents.offense += 1;
    state.player.attackBonus += 1;
  } else if (type === "guard") {
    state.talents.guard += 1;
    state.player.defenseBonus += 1;
  } else if (type === "support") {
    state.talents.support += 1;
    state.player.maxMp += 4;
    state.player.mp = Math.min(state.player.maxMp, state.player.mp + 4);
  }
  addJournal("\u5929\u8d4b\u63d0\u5347: " + ({ offense: "\u8f93\u51fa", guard: "\u9632\u5fa1", support: "\u652f\u63f4" }[type] || type));
  renderAll();
}

function autoAllocateTalents() {
  ensureGameplayState();
  while ((state.talents.points || 0) > 0) {
    const arr = ["offense", "guard", "support"];
    applyTalent(arr[Math.floor(Math.random() * arr.length)]);
  }
}

function formatTalentSummary() {
  return "\u5929\u8d4b: \u8f93\u51fa" + state.talents.offense + " \u9632\u5fa1" + state.talents.guard + " \u652f\u63f4" + state.talents.support + " | \u70b9\u6570 " + state.talents.points;
}

function showTalentSummary() {
  if (refs.skillSummary) refs.skillSummary.textContent = formatTalentSummary();
}

function clearBossTimeline() {
  state.combat.timeline = [];
  renderBoss();
}

function resetBuild() {
  state.player.attackBonus = 0;
  state.player.defenseBonus = 0;
  state.talents.offense = 0;
  state.talents.guard = 0;
  state.talents.support = 0;
  showToast("\u5df2\u91cd\u7f6e\u5929\u8d4b\u4e0e\u6784\u7b51");
  renderAll();
}

function castSelectedSkill() {
  const skill = refs.skillSelect?.value || "quick_strike";
  const skillDef = SKILL_LIBRARY.find((x) => x.id === skill) || SKILL_LIBRARY[0];
  const mpCost = skillDef.mp || 10;
  if (state.player.mp < mpCost) return showToast("\u6cd5\u529b\u4e0d\u8db3");
  state.player.mp = Math.max(0, state.player.mp - mpCost);

  if (skillDef.kind === "heal") {
    const heal = 16 + state.player.level * 2 + state.talents.support * 2;
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + heal);
    if (refs.skillSummary) refs.skillSummary.textContent = skillDef.name + " \u6062\u590d " + heal + " \u70b9\u751f\u547d";
    addJournal("\u65bd\u653e " + skillDef.name + "\uff0c\u6062\u590d\u751f\u547d " + heal);
  } else if (skillDef.kind === "shield") {
    const shield = 1 + Math.floor(state.player.level / 3);
    state.player.defenseBonus += shield;
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + 6);
    if (refs.skillSummary) refs.skillSummary.textContent = skillDef.name + "\uff1a\u9632\u5fa1\u63d0\u5347 +" + shield + "\uff0c\u5e76\u6062\u590d\u5c11\u91cf\u751f\u547d";
    addJournal("\u65bd\u653e " + skillDef.name + "\uff0c\u9632\u5fa1\u63d0\u5347 " + shield);
  } else if (skillDef.id === "wind_grace") {
    const regen = 8 + state.talents.support * 2;
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + regen);
    state.player.mp = Math.min(state.player.maxMp, state.player.mp + 6);
    if (refs.skillSummary) refs.skillSummary.textContent = skillDef.name + "\uff1a\u751f\u547d+" + regen + "\uff0c\u6cd5\u529b+6";
    addJournal("\u65bd\u653e " + skillDef.name + "\uff0c\u8fdb\u5165\u7eed\u822a\u59ff\u6001\u3002");
  } else if (skillDef.id === "chrono_anchor") {
    const guard = 2 + Math.floor(state.talents.support / 2);
    state.player.defenseBonus += guard;
    state.player.mp = Math.min(state.player.maxMp, state.player.mp + 10);
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + 10);
    addMissionMetric("boss_hits", 1);
    if (refs.skillSummary) refs.skillSummary.textContent = skillDef.name + "\uff1a\u9632\u5fa1+" + guard + "\uff0c\u751f\u547d+10\uff0c\u6cd5\u529b+10";
    addJournal("\u65bd\u653e " + skillDef.name + "\uff0c\u7a33\u5b9a\u5f53\u524d\u6218\u7ebf\u8282\u594f\u3002");
  } else {
    const base = 10 + state.player.level * 3 + state.talents.offense * 2;
    let damage = base + (skillDef.bonus || 6) + Math.floor(Math.random() * 10);
    if (skillDef.id === "lunar_rain") damage += 12 + state.talents.support;
    if (skillDef.id === "shadow_step") state.player.hp = Math.min(state.player.maxHp, state.player.hp + 4);
    if (skillDef.id === "thunder_spear" || skillDef.id === "starfall" || skillDef.id === "lunar_rain") {
      progressSideQuests("combat", 2);
    } else {
      progressSideQuests("combat", 1);
    }
    if (skillDef.id === "blazing_arc" || skillDef.id === "lunar_rain") progressSideQuests("explore", 1);
    if (refs.skillSummary) refs.skillSummary.textContent = "\u65bd\u653e " + skillDef.name + "\uff0c\u9020\u6210 " + damage + " \u70b9\u4f24\u5bb3";
    addJournal("\u6280\u80fd\u547d\u4e2d\uff1a" + skillDef.name + " \u9020\u6210 " + damage + " \u4f24\u5bb3");
  }

  renderAll();
}

function castCompanionSkill() {
  const mate = activeCompanion();
  if (!mate) return showToast("\u5f53\u524d\u6ca1\u6709\u53ef\u7528\u540c\u4f34");

  let summary = "";
  if (mate.key === "guardian") {
    const guard = 2 + mate.def + Math.floor(mate.level / 2);
    const heal = 6 + mate.support * 2 + mate.bond;
    state.player.defenseBonus += guard;
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + heal);
    progressSideQuests("explore", 1);
    summary = mate.name + " \u65bd\u653e " + mate.skillName + "\uff0c\u62a4\u7532 +" + guard + "\uff0c\u751f\u547d +" + heal;
  } else if (mate.key === "ranger") {
    const damage = 12 + mate.atk * 4 + mate.level * 2 + mate.bond;
    progressSideQuests("combat", 1);
    progressSideQuests("explore", 1);
    summary = mate.name + " \u65bd\u653e " + mate.skillName + "\uff0c\u9020\u6210 " + damage + " \u70b9\u8f85\u52a9\u4f24\u5bb3\u3002";
  } else if (mate.key === "mystic") {
    const heal = 10 + mate.support * 4 + mate.level;
    const mana = 8 + mate.support * 2 + mate.level;
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + heal);
    state.player.mp = Math.min(state.player.maxMp, state.player.mp + mana);
    progressSideQuests("artifact", 1);
    summary = mate.name + " \u65bd\u653e " + mate.skillName + "\uff0c\u751f\u547d +" + heal + "\uff0c\u6cd5\u529b +" + mana;
  } else {
    const damage = 16 + mate.atk * 5 + mate.level * 2 + mate.bond;
    const guard = 1 + Math.floor(mate.def / 2);
    state.player.defenseBonus += guard;
    progressSideQuests("combat", 2);
    summary = mate.name + " \u65bd\u653e " + mate.skillName + "\uff0c\u9020\u6210 " + damage + " \u70b9\u4f24\u5bb3\uff0c\u5e76\u63d0\u5347 " + guard + " \u70b9\u9632\u5fa1\u3002";
  }

  grantCompanionGrowth("\u540c\u4f34\u6280\u80fd", 10, 1, { companionId: mate.id, log: false, toast: false });
  if (refs.skillSummary) refs.skillSummary.textContent = summary + "\n\n" + activeCompanionSummary();
  addJournal(summary);
  renderAll();
}

function getDecisionBossModifiers(chapterId) {
  const out = {
    damageMul: 1,
    bossHpMul: 1,
    counterBonus: 0,
    reflectBonus: 0,
    cadenceLockout: false,
    periodicBonusDamage: 0,
    selfDrainOnBurst: 0
  };

  if (chapterId === "camp") {
    const pick = getDecisionById("camp_supply_route");
    if (pick === "safe_pass") {
      out.damageMul = 1.08;
      out.reflectBonus = -0.02;
    } else if (pick === "rift_shortcut") {
      out.bossHpMul = 0.9;
      out.counterBonus = 3;
    }
  } else if (chapterId === "forest") {
    const pick = getDecisionById("forest_shrine");
    if (pick === "seal_shrine") {
      out.cadenceLockout = true;
      out.reflectBonus = -0.03;
    } else if (pick === "draw_power") {
      out.periodicBonusDamage = 12;
      out.selfDrainOnBurst = 4;
    }
  } else if (chapterId === "rift") {
    const pick = getDecisionById("rift_alliance");
    if (pick === "wardens_line") {
      out.counterBonus = -3;
    } else if (pick === "arcanum_ritual") {
      out.periodicBonusDamage = 18;
    } else if (pick === "freeguild_charge") {
      out.damageMul = 1.12;
      out.reflectBonus = 0.02;
    }
  }
  return out;
}

function triggerBossMapEffect(kind) {
  if (!refs.worldMap) return;
  refs.worldMap.classList.remove("boss-pulse", "boss-shake", "boss-rift");
  void refs.worldMap.offsetWidth;
  const cls = kind === "shake" ? "boss-shake" : kind === "rift" ? "boss-rift" : "boss-pulse";
  refs.worldMap.classList.add(cls);
  clearTimeout(triggerBossMapEffect.timer);
  triggerBossMapEffect.timer = setTimeout(() => refs.worldMap?.classList.remove("boss-pulse", "boss-shake", "boss-rift"), 380);
}

function startBoss() {
  ensureGameplayState();
  state.combat.active = true;
  const id = currentChapterId();
  const profile = BOSS_PROFILES[id] || BOSS_PROFILES.rift;
  const mod = getDecisionBossModifiers(id);
  const reflect = Math.max(0.02, profile.reflect + mod.reflectBonus);
  state.combat.profile = profile;
  state.combat.mod = mod;
  state.combat.reflect = reflect;
  state.combat.bossName = profile.name;
  state.combat.maxHp = Math.max(60, Math.floor((profile.hpBase + state.player.level * profile.hpPerLevel) * mod.bossHpMul));
  state.combat.hp = state.combat.maxHp;
  state.combat.turn = 1;
  state.combat.timeline = ["\u906d\u9047 Boss: " + state.combat.bossName];

  if (id === "camp") {
    state.combat.timeline.push("\u673a\u5236\uff1a\u6bcf 3 \u56de\u5408\u89e6\u53d1\u575a\u76fe\u3002\u5e94\u5bf9\uff1a\u8f93\u51fa\u5929\u8d4b>=2 \u6216\u653b\u51fb\u52a0\u6210>=4\u3002");
  } else if (id === "forest") {
    state.combat.timeline.push("\u673a\u5236\uff1a\u6728\u76fe\u6309\u8282\u594f\u751f\u6548\u3002\u5e94\u5bf9\uff1a\u4fdd\u7559\u788e\u7247\u7528\u4e8e\u7834\u76fe\u3002");
  } else {
    state.combat.timeline.push("\u673a\u5236\uff1a\u88c2\u9699\u626d\u66f2\u538b\u5236\u8282\u594f\u3002\u5e94\u5bf9\uff1a\u589e\u52a0\u540c\u4f34\u6216\u63d0\u5347\u652f\u63f4\u5929\u8d4b\u3002");
  }

  if (mod.damageMul > 1) state.combat.timeline.push("\u51b3\u7b56\u589e\u76ca\uff1a\u4f24\u5bb3\u63d0\u5347");
  if (mod.bossHpMul < 1) state.combat.timeline.push("\u51b3\u7b56\u589e\u76ca\uff1aBoss \u751f\u547d\u4e0b\u964d");
  if (mod.periodicBonusDamage > 0) state.combat.timeline.push("\u51b3\u7b56\u589e\u76ca\uff1a\u89e6\u53d1\u5468\u671f\u7206\u53d1");
  showToast("Boss \u6218\u5df2\u5f00\u59cb");
  renderBoss();
}

function strikeBoss() {
  if (!state.combat.active) return showToast("\u8bf7\u5148\u5f00\u542f Boss \u6218");
  const profile = state.combat.profile || BOSS_PROFILES.rift;
  const mod = state.combat.mod || getDecisionBossModifiers(currentChapterId());
  const chapterId = currentChapterId();
  const turn = state.combat.turn || 1;

  let dmg = Math.max(8, state.player.baseAttack + state.player.attackBonus + Math.floor(Math.random() * 10));
  dmg = Math.floor(dmg * (mod.damageMul || 1));
  let extraCounter = 0;

  if (chapterId === "camp" && turn % 3 === 0) {
    const canBreak = (state.talents?.offense || 0) >= 2 || (state.player.attackBonus || 0) >= 4;
    if (canBreak) {
      dmg = Math.floor(dmg * 1.18);
      state.combat.timeline.push("\u8425\u5730\uff1a\u575a\u76fe\u5df2\u88ab\u51fb\u7a7f\uff0c\u8fdb\u5165\u8f93\u51fa\u7a97\u53e3\u3002");
      triggerBossMapEffect("pulse");
    } else {
      dmg = Math.floor(dmg * 0.72);
      extraCounter += 2;
      state.combat.timeline.push("\u8425\u5730\uff1a\u575a\u76fe\u538b\u5236\u4e86\u4f60\u7684\u4f24\u5bb3\uff0c\u8bf7\u63d0\u524d\u5806\u53e0\u8f93\u51fa\u3002");
      triggerBossMapEffect("shake");
    }
  }

  if (chapterId === "forest" && turn % profile.cadence === 0) {
    if (mod.cadenceLockout) {
      state.combat.timeline.push("\u68ee\u6797\uff1a\u51b3\u7b56\u6548\u679c\u5df2\u5c01\u9501\u6728\u76fe\u8282\u594f\u3002");
      triggerBossMapEffect("pulse");
    } else if (state.content.artifactShards > 0) {
      state.content.artifactShards -= 1;
      dmg = Math.floor(dmg * 1.1);
      state.combat.timeline.push("\u68ee\u6797\uff1a\u6d88\u8017 1 \u788e\u7247\u7a7f\u900f\u4e86\u6728\u76fe\u3002");
      triggerBossMapEffect("pulse");
    } else {
      dmg = Math.floor(dmg * 0.72);
      state.combat.timeline.push("\u68ee\u6797\uff1a\u6728\u76fe\u5438\u6536\u4e86\u4f24\u5bb3\uff0c\u8bf7\u5148\u6536\u96c6\u788e\u7247\u3002");
      triggerBossMapEffect("shake");
    }
  }

  if (chapterId === "rift" && turn % 3 === 0) {
    const synced = (state.party?.roster?.length || 0) >= 2 || (state.talents?.support || 0) >= 2;
    if (synced) {
      dmg = Math.floor(dmg * 1.08);
      state.combat.timeline.push("\u88c2\u9699\uff1a\u534f\u540c\u8282\u594f\u7a33\u5b9a\uff0c\u4f60\u7684\u653b\u51fb\u66f4\u5e73\u6ed1\u3002");
      triggerBossMapEffect("pulse");
    } else {
      dmg = Math.floor(dmg * 0.62);
      extraCounter += 3;
      state.combat.timeline.push("\u88c2\u9699\uff1a\u626d\u66f2\u538b\u5236\u4e86\u653b\u51fb\uff0c\u8bf7\u589e\u52a0\u540c\u4f34\u6216\u652f\u63f4\u5929\u8d4b\u3002");
      triggerBossMapEffect("rift");
    }
  }

  if (mod.periodicBonusDamage > 0 && turn % 2 === 0) {
    dmg += mod.periodicBonusDamage;
    state.combat.timeline.push("\u51b3\u7b56\u7206\u53d1\uff1a+" + mod.periodicBonusDamage + " \u4f24\u5bb3");
    if (mod.selfDrainOnBurst > 0) {
      state.player.hp = Math.max(1, state.player.hp - mod.selfDrainOnBurst);
      state.combat.timeline.push("\u53cd\u566c\uff1a-" + mod.selfDrainOnBurst + " HP");
    }
  }

  addMissionMetric("boss_hits", 1);
  state.combat.hp = Math.max(0, state.combat.hp - dmg);
  state.combat.timeline.push("\u4f60\u9020\u6210\u4f24\u5bb3 " + dmg);

  if (state.combat.hp <= 0) {
    state.combat.timeline.push("Boss \u5df2\u88ab\u51fb\u8d25");
    state.combat.active = false;
    addMissionMetric("boss_kills", 1);
    state.story.progress.boss += 1;
    state.content.artifactShards += 2;
    progressSideQuests("combat", 1);
    gain(120, 180);
    addJournal("\u51fb\u8d25 Boss: " + state.combat.bossName);
    showToast("Boss \u5df2\u88ab\u51fb\u8d25");
    renderAll();
    return;
  }

  const reflectRate = Math.max(0.01, Number(state.combat.reflect || profile.reflect || 0.1));
  const reflected = Math.max(1, Math.floor(dmg * reflectRate));
  let ret = Math.max(1, 6 + Math.floor(Math.random() * 12) + reflected + (mod.counterBonus || 0) + extraCounter);

  if (state.combat.hp <= state.combat.maxHp * 0.4 && turn % 2 === 0) {
    ret += 3;
    state.combat.timeline.push("\u72c2\u66b4\u6ce2\u52a8\uff1a\u53cd\u51fb\u5f3a\u5ea6\u63d0\u5347\u3002");
    triggerBossMapEffect("shake");
  }

  state.player.hp = Math.max(1, state.player.hp - ret);
  state.combat.timeline.push("Boss \u53cd\u51fb\u9020\u6210\u4f24\u5bb3 " + ret);
  if (ret >= 14) triggerBossMapEffect("shake");
  state.combat.turn = turn + 1;
  renderAll();
}

async function runDamageSimulation() {
  try {
    const res = await gameServices.calculateDamage({
      attacker_id: Number(refs.inputAttacker?.value || 1),
      defender_id: Number(refs.inputDefender?.value || 2),
      skill_id: Number(refs.inputSkill?.value || 1)
    });
    refs.combatResult && (refs.combatResult.textContent = `\u4f24\u5bb3 ${res.damage} | \u66b4\u51fb ${res.is_critical ? "\u662f" : "\u5426"} | ${res.effect}`);
  } catch (e) {
    refs.combatResult && (refs.combatResult.textContent = `\u4f24\u5bb3\u8ba1\u7b97\u5931\u8d25: ${e.details || e.message}`);
  }
}

async function runAIDecision() {
  try {
    const res = await gameServices.aiDecision({ npc_id: Number(refs.inputDefender?.value || 2), context: [state.world.day, state.player.hp] });
    refs.combatResult && (refs.combatResult.textContent = `AI \u52a8\u4f5c ${res.action_id} | \u7f6e\u4fe1\u5ea6 ${Number(res.confidence || 0).toFixed(2)} | ${res.description}`);
  } catch (e) {
    refs.combatResult && (refs.combatResult.textContent = `AI \u51b3\u7b56\u5931\u8d25: ${e.details || e.message}`);
  }
}

window.FantasyCombatModule = Object.freeze({
  renderBoss,
  renderPartyAndSkills,
  formatTalentSummary,
  showTalentSummary,
  applyTalent,
  autoAllocateTalents,
  resetBuild,
  clearBossTimeline,
  getDecisionBossModifiers,
  triggerBossMapEffect,
  startBoss,
  strikeBoss,
  castSelectedSkill,
  castCompanionSkill,
  runDamageSimulation,
  runAIDecision
});
