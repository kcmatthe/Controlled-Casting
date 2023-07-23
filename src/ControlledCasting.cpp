
#include "ControlledCasting.h"
#include "PCH.h"
#include "config.h"
#include "Cache.h"

namespace config
{
	std::vector<ControlledCasting::Override*> overrides;
	std::vector<ControlledCasting::Multiplier*> multipliers;
	std::vector<ControlledCasting::Modifier*> modifiers;

	
	bool GetFromJSON()
	{
		const char* editorID;
		bool exclude;
		bool bOverride;
		float value;

		logger::info("Attempting to access ControlledCasting.json");
		
		std::filesystem::path configPath{ "Data"sv };
		auto configLocation = configPath / "ControlledCasting.json"sv;
		::errno_t err = 0;
		std::unique_ptr<FILE, decltype(&::fclose)> fp{
			[&configLocation, &err] {
				FILE* fp = nullptr;
				err = _wfopen_s(&fp, configLocation.c_str(), L"r");
				return fp;
			}(),
			&::fclose
		};
		if (err != 0) {
			logger::warn("Failed to open file: {}", configLocation.string());
		} else {
			char readBuffer[65536]{};
			rapidjson::FileReadStream is{ fp.get(), readBuffer, sizeof(readBuffer) };
			rapidjson::Document config;

			config.ParseStream(is);

			if (config.HasParseError()) {
				logger::info("parse error");
			} else {
				logger::info("no parse error");
			}
			rapidjson::StringBuffer configBuffer{};
			rapidjson::Writer<rapidjson::StringBuffer> writer{ configBuffer };
			config.Accept(writer);
			const std::string configStr{ configBuffer.GetString() };
			logger::info("Json string: {}", configStr);

			if (config.HasMember("Spells")) {
				const rapidjson::Value& spells = config["Spells"];
				if (spells.IsArray()) {
					logger::info("About to iterate through all {} spells in the json", spells.Size());
					for (int i = 0; i < spells.Size(); i++) {
						if (spells[i].HasMember("editorID") && spells[i]["editorID"].IsString()) {
							editorID = spells[i]["editorID"].GetString();
						} else {
							logger::info("could not find valid editorID in JSON for spell index {}", i);
							editorID = nullptr;
						}
						if (editorID == nullptr) {
							logger::info("no valid editor ID for spell index {}", i);
						} else {
							if (spells[i].HasMember("excluded") && spells[i]["excluded"].IsBool()) {
								exclude = (spells[i]["excluded"].GetBool());
							} else {
								exclude = false;
								logger::info("No valid exclusion for spell index {}. Exclude set to false", i);
							}
							if (spells[i].HasMember("override") && spells[i]["override"].IsBool()) {
								bOverride = (spells[i]["override"].GetBool());
							} else {
								bOverride = false;
								logger::info("No valid override for spell index {}. Override set to false", i);
							}
							if (spells[i].HasMember("value") && spells[i]["value"].IsFloat()) {
								value = spells[i]["value"].GetFloat();
							} else {
								if (bOverride) {
									value = 0.5;
									logger::info("Override was set to true but no valid value was found for spell index {}. Value set to 0.5.", i);
								}
							}

							std::string string(editorID);
							RE::SpellItem* spell = nullptr;
							/* auto& allSpells = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::SpellItem>();
							for (auto s : allSpells) {
								if (s->GetFormEditorID() == editorID) {
									logger::info("editorID was a match {}", string);
									spell = RE::TESForm::LookupByEditorID(string)->As<RE::SpellItem>();
									break;
								} 
							}*/
							spell = RE::TESForm::LookupByEditorID(string)->As<RE::SpellItem>();
							if (spell != nullptr) {
								logger::info("Read spell {}, override: {}, exclude: {}, value {}", spell->fullName.c_str(), bOverride, exclude, value);
								ControlledCasting::Override* override = new ControlledCasting::Override(spell, exclude, bOverride, value);
								overrides.push_back(override);
							} else {
								logger::info("{} is not a valid editorID", string);
							}
						}
					}
				} else {
					logger::info("ControlledCasting.JSON did not contain a valid array: Spells");
				}
			} else {
				logger::info("ControlledCasting.JSON did not contain a valid object: Spells");
			}
		}
		return true;
	}
}

namespace ControlledCasting {
	
	//Global Variables - can be adjusted via MCM
	//Player
	RE::TESGlobal* ShowMsgs = nullptr;
	
	RE::TESGlobal* FFChargeTimeMult = nullptr;
	RE::TESGlobal* ConcChargeTimeMod = nullptr;

	RE::TESGlobal* RitualFFChargeTimeMult = nullptr;
	RE::TESGlobal* RitualConcChargeTimeMod = nullptr;
	RE::TESGlobal* RitualDefaultFFOverride = nullptr;
	RE::TESGlobal* RitualDefaultConcOverride = nullptr;
	RE::TESGlobal* RitualDefaultFFOverrideToggle = nullptr;
	RE::TESGlobal* RitualDefaultConcOverrideToggle = nullptr;

	RE::TESGlobal* AlterationFFChargeTimeMult = nullptr;
	RE::TESGlobal* AlterationConcChargeTimeMod = nullptr;
	RE::TESGlobal* DestructionFFChargeTimeMult = nullptr;
	RE::TESGlobal* DestructionConcChargeTimeMod = nullptr;
	RE::TESGlobal* ConjurationFFChargeTimeMult = nullptr;
	RE::TESGlobal* ConjurationConcChargeTimeMod = nullptr;
	RE::TESGlobal* IllusionFFChargeTimeMult = nullptr;
	RE::TESGlobal* IllusionConcChargeTimeMod = nullptr;
	RE::TESGlobal* RestorationFFChargeTimeMult = nullptr;
	RE::TESGlobal* RestorationConcChargeTimeMod = nullptr;

	RE::TESGlobal* NoviceFFChargeTimeMult = nullptr;
	RE::TESGlobal* NoviceConcChargeTimeMod = nullptr;
	RE::TESGlobal* ApprenticeFFChargeTimeMult = nullptr;
	RE::TESGlobal* ApprenticeConcChargeTimeMod = nullptr;
	RE::TESGlobal* AdeptFFChargeTimeMult = nullptr;
	RE::TESGlobal* AdeptConcChargeTimeMod = nullptr;
	RE::TESGlobal* ExpertFFChargeTimeMult = nullptr;
	RE::TESGlobal* ExpertConcChargeTimeMod = nullptr;
	RE::TESGlobal* MasterFFChargeTimeMult = nullptr;
	RE::TESGlobal* MasterConcChargeTimeMod = nullptr;

	RE::TESGlobal* NoviceDefaultFFOverride = nullptr;
	RE::TESGlobal* ApprenticeDefaultFFOverride = nullptr;
	RE::TESGlobal* AdeptDefaultFFOverride = nullptr;
	RE::TESGlobal* ExpertDefaultFFOverride = nullptr;
	RE::TESGlobal* MasterDefaultFFOverride = nullptr;

	RE::TESGlobal* NoviceDefaultFFOverrideToggle = nullptr;
	RE::TESGlobal* ApprenticeDefaultFFOverrideToggle = nullptr;
	RE::TESGlobal* AdeptDefaultFFOverrideToggle = nullptr;
	RE::TESGlobal* ExpertDefaultFFOverrideToggle = nullptr;
	RE::TESGlobal* MasterDefaultFFOverrideToggle = nullptr;

	RE::TESGlobal* NoviceDefaultConcOverride = nullptr;
	RE::TESGlobal* ApprenticeDefaultConcOverride = nullptr;
	RE::TESGlobal* AdeptDefaultConcOverride = nullptr;
	RE::TESGlobal* ExpertDefaultConcOverride = nullptr;
	RE::TESGlobal* MasterDefaultConcOverride = nullptr;

	RE::TESGlobal* NoviceDefaultConcOverrideToggle = nullptr;
	RE::TESGlobal* ApprenticeDefaultConcOverrideToggle = nullptr;
	RE::TESGlobal* AdeptDefaultConcOverrideToggle = nullptr;
	RE::TESGlobal* ExpertDefaultConcOverrideToggle = nullptr;
	RE::TESGlobal* MasterDefaultConcOverrideToggle = nullptr;

	//NPCS
	RE::TESGlobal* FFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* ConcChargeTimeMod_NPC = nullptr;

	RE::TESGlobal* AlterationFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* AlterationConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* DestructionFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* DestructionConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* ConjurationFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* ConjurationConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* IllusionFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* IllusionConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* RestorationFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* RestorationConcChargeTimeMod_NPC = nullptr;

	RE::TESGlobal* NoviceFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* NoviceConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* ApprenticeFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* ApprenticeConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* AdeptFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* AdeptConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* ExpertFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* ExpertConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* MasterFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* MasterConcChargeTimeMod_NPC = nullptr;

	RE::TESGlobal* NoviceDefaultFFOverride_NPC = nullptr;
	RE::TESGlobal* ApprenticeDefaultFFOverride_NPC = nullptr;
	RE::TESGlobal* AdeptDefaultFFOverride_NPC = nullptr;
	RE::TESGlobal* ExpertDefaultFFOverride_NPC = nullptr;
	RE::TESGlobal* MasterDefaultFFOverride_NPC = nullptr;

	RE::TESGlobal* NoviceDefaultFFOverrideToggle_NPC = nullptr;
	RE::TESGlobal* ApprenticeDefaultFFOverrideToggle_NPC = nullptr;
	RE::TESGlobal* AdeptDefaultFFOverrideToggle_NPC = nullptr;
	RE::TESGlobal* ExpertDefaultFFOverrideToggle_NPC = nullptr;
	RE::TESGlobal* MasterDefaultFFOverrideToggle_NPC = nullptr;

	RE::TESGlobal* NoviceDefaultConcOverride_NPC = nullptr;
	RE::TESGlobal* ApprenticeDefaultConcOverride_NPC = nullptr;
	RE::TESGlobal* AdeptDefaultConcOverride_NPC = nullptr;
	RE::TESGlobal* ExpertDefaultConcOverride_NPC = nullptr;
	RE::TESGlobal* MasterDefaultConcOverride_NPC = nullptr;

	RE::TESGlobal* NoviceDefaultConcOverrideToggle_NPC = nullptr;
	RE::TESGlobal* ApprenticeDefaultConcOverrideToggle_NPC = nullptr;
	RE::TESGlobal* AdeptDefaultConcOverrideToggle_NPC = nullptr;
	RE::TESGlobal* ExpertDefaultConcOverrideToggle_NPC = nullptr;
	RE::TESGlobal* MasterDefaultConcOverrideToggle_NPC = nullptr;

	RE::TESGlobal* RitualFFChargeTimeMult_NPC = nullptr;
	RE::TESGlobal* RitualConcChargeTimeMod_NPC = nullptr;
	RE::TESGlobal* RitualDefaultFFOverride_NPC = nullptr;
	RE::TESGlobal* RitualDefaultConcOverride_NPC = nullptr;
	RE::TESGlobal* RitualDefaultFFOverrideToggle_NPC = nullptr;
	RE::TESGlobal* RitualDefaultConcOverrideToggle_NPC = nullptr;


	float GetChargeTimeVHook::thunk(RE::SpellItem* spell)
	{
		auto casting = spell->GetCastingType();
		float time = func(spell);  
		float debug = *reinterpret_cast<float*>(&ShowMsgs->value);
		if (casting == RE::MagicSystem::CastingType::kFireAndForget || casting == RE::MagicSystem::CastingType::kConcentration) {
			int numeffects = spell->effects.size();
			int32_t difficulty;
			bool ritual;
			RE::ActorValue school;  //= RE::ActorValue::kAlteration;
			
			for (Override* override : config::overrides) {
				if (override->spell == spell) {
					if (override->excluded) {
						if (override->override) {
							if (debug == 1) {
								logger::info("exclusion and override detected, returning override value");
							}
							return override->value;
						} else {
							if (debug == 1) {
								logger::info("exclusion detected, returning original charge time");
							}
							return func(spell);
						}
					} else {
						if (override->override)
							if (debug == 1) {
								logger::info("override detected, returning value from config");
							}
						time = override->value;
						break;
					}
				}
			}
			if (numeffects > 0) {
				school = spell->GetCostliestEffectItem()->baseEffect->GetMagickSkill();
				difficulty = spell->GetCostliestEffectItem()->baseEffect->GetMinimumSkillLevel();
				ritual = spell->GetCostliestEffectItem()->baseEffect->HasKeywordString("RitualSpellEffect");
			} else {
				return time;
			}
			auto PlayerRef = RE::PlayerCharacter::GetSingleton();

			//How to introduce and utilize custom multipliers and modifiers
			/* float customMultiplier = 1;
			float customModifier = 0;

			for (auto* multiplier : config::multipliers) {
				float tempMult = multiplier->value;
				customMultiplier = customMultiplier * tempMult;
			}*/

		
			//floats
			float FFmult_Main;
			float FFmult_Difficulty = 1;
			float FFmult_School = 1;
			float FFmult_Novice;
			float FFmult_Apprentice;
			float FFmult_Adept;
			float FFmult_Expert;
			float FFmult_Master;
			float FFmult_Alteration;
			float FFmult_Conjuration;
			float FFmult_Destruction;
			float FFmult_Illusion;
			float FFmult_Restoration;
			float FFmult_Ritual;

			float ConcMod_Main;
			float ConcMod_Difficulty = 0;
			float ConcMod_School = 0;
			float ConcMod_Novice;
			float ConcMod_Apprentice;
			float ConcMod_Adept;
			float ConcMod_Expert;
			float ConcMod_Master;
			float ConcMod_Alteration;
			float ConcMod_Conjuration;
			float ConcMod_Destruction;
			float ConcMod_Illusion;
			float ConcMod_Restoration;
			float ConcMod_Ritual;

			float FFoverride_Difficulty = 0.5;
			float FFoverride_Novice;
			float FFoverride_Apprentice;
			float FFoverride_Adept;
			float FFoverride_Expert;
			float FFoverride_Master;
			float FFoverride_Ritual;

			float ConcOverride_Difficulty = 0;
			float ConcOverride_Novice;
			float ConcOverride_Apprentice;
			float ConcOverride_Adept;
			float ConcOverride_Expert;
			float ConcOverride_Master;
			float ConcOverride_Ritual;

			float FFoverride_Toggle_Difficulty = 0;
			float FFoverride_Toggle_Novice;
			float FFoverride_Toggle_Apprentice;
			float FFoverride_Toggle_Adept;
			float FFoverride_Toggle_Expert;
			float FFoverride_Toggle_Master;
			float FFoverride_Toggle_Ritual;

			float ConcOverride_Toggle_Difficulty = 0;
			float ConcOverride_Toggle_Novice;
			float ConcOverride_Toggle_Apprentice;
			float ConcOverride_Toggle_Adept;
			float ConcOverride_Toggle_Expert;
			float ConcOverride_Toggle_Master;
			float ConcOverride_Toggle_Ritual;
		
		

				if (PlayerRef->GetEquippedObject(true) == spell || PlayerRef->GetEquippedObject(false) == spell) {
					FFmult_Main = *reinterpret_cast<float*>(&FFChargeTimeMult->value);
					FFmult_Novice = *reinterpret_cast<float*>(&NoviceFFChargeTimeMult->value);
					FFmult_Apprentice = *reinterpret_cast<float*>(&ApprenticeFFChargeTimeMult->value);
					FFmult_Adept = *reinterpret_cast<float*>(&AdeptFFChargeTimeMult->value);
					FFmult_Expert = *reinterpret_cast<float*>(&ExpertFFChargeTimeMult->value);
					FFmult_Master = *reinterpret_cast<float*>(&MasterFFChargeTimeMult->value);
					FFmult_Alteration = *reinterpret_cast<float*>(&AlterationFFChargeTimeMult->value);
					FFmult_Conjuration = *reinterpret_cast<float*>(&ConjurationFFChargeTimeMult->value);
					FFmult_Destruction = *reinterpret_cast<float*>(&DestructionFFChargeTimeMult->value);
					FFmult_Illusion = *reinterpret_cast<float*>(&IllusionFFChargeTimeMult->value);
					FFmult_Restoration = *reinterpret_cast<float*>(&RestorationFFChargeTimeMult->value);
					FFmult_Ritual = *reinterpret_cast<float*>(&RitualFFChargeTimeMult->value);

					ConcMod_Main = *reinterpret_cast<float*>(&ConcChargeTimeMod->value);
					ConcMod_Novice = *reinterpret_cast<float*>(&NoviceConcChargeTimeMod->value);
					ConcMod_Apprentice = *reinterpret_cast<float*>(&ApprenticeConcChargeTimeMod->value);
					ConcMod_Adept = *reinterpret_cast<float*>(&AdeptConcChargeTimeMod->value);
					ConcMod_Expert = *reinterpret_cast<float*>(&ExpertConcChargeTimeMod->value);
					ConcMod_Master = *reinterpret_cast<float*>(&MasterConcChargeTimeMod->value);
					ConcMod_Alteration = *reinterpret_cast<float*>(&AlterationConcChargeTimeMod->value);
					ConcMod_Conjuration = *reinterpret_cast<float*>(&ConjurationConcChargeTimeMod->value);
					ConcMod_Destruction = *reinterpret_cast<float*>(&DestructionConcChargeTimeMod->value);
					ConcMod_Illusion = *reinterpret_cast<float*>(&IllusionConcChargeTimeMod->value);
					ConcMod_Restoration = *reinterpret_cast<float*>(&RestorationConcChargeTimeMod->value);
					ConcMod_Ritual = *reinterpret_cast<float*>(&RitualConcChargeTimeMod->value);

					FFoverride_Novice = *reinterpret_cast<float*>(&NoviceDefaultFFOverride->value);
					FFoverride_Apprentice = *reinterpret_cast<float*>(&ApprenticeDefaultFFOverride->value);
					FFoverride_Adept = *reinterpret_cast<float*>(&AdeptDefaultFFOverride->value);
					FFoverride_Expert = *reinterpret_cast<float*>(&ExpertDefaultFFOverride->value);
					FFoverride_Master = *reinterpret_cast<float*>(&MasterDefaultFFOverride->value);
					FFoverride_Ritual = *reinterpret_cast<float*>(&RitualDefaultFFOverride->value);

					ConcOverride_Novice = *reinterpret_cast<float*>(&NoviceDefaultConcOverride->value);
					ConcOverride_Apprentice = *reinterpret_cast<float*>(&ApprenticeDefaultConcOverride->value);
					ConcOverride_Adept = *reinterpret_cast<float*>(&AdeptDefaultConcOverride->value);
					ConcOverride_Expert = *reinterpret_cast<float*>(&ExpertDefaultConcOverride->value);
					ConcOverride_Master = *reinterpret_cast<float*>(&MasterDefaultConcOverride->value);
					ConcOverride_Ritual = *reinterpret_cast<float*>(&RitualDefaultConcOverride->value);

					FFoverride_Toggle_Novice = *reinterpret_cast<float*>(&NoviceDefaultFFOverrideToggle->value);
					FFoverride_Toggle_Apprentice = *reinterpret_cast<float*>(&ApprenticeDefaultFFOverrideToggle->value);
					FFoverride_Toggle_Adept = *reinterpret_cast<float*>(&AdeptDefaultFFOverrideToggle->value);
					FFoverride_Toggle_Expert = *reinterpret_cast<float*>(&ExpertDefaultFFOverrideToggle->value);
					FFoverride_Toggle_Master = *reinterpret_cast<float*>(&MasterDefaultFFOverrideToggle->value);
					FFoverride_Toggle_Ritual = *reinterpret_cast<float*>(&RitualDefaultFFOverrideToggle->value);

					ConcOverride_Toggle_Novice = *reinterpret_cast<float*>(&NoviceDefaultConcOverrideToggle->value);
					ConcOverride_Toggle_Apprentice = *reinterpret_cast<float*>(&ApprenticeDefaultConcOverrideToggle->value);
					ConcOverride_Toggle_Adept = *reinterpret_cast<float*>(&AdeptDefaultConcOverrideToggle->value);
					ConcOverride_Toggle_Expert = *reinterpret_cast<float*>(&ExpertDefaultConcOverrideToggle->value);
					ConcOverride_Toggle_Master = *reinterpret_cast<float*>(&MasterDefaultConcOverrideToggle->value);
					ConcOverride_Toggle_Ritual = *reinterpret_cast<float*>(&RitualDefaultConcOverrideToggle->value);
				} else {
					FFmult_Main = *reinterpret_cast<float*>(&FFChargeTimeMult_NPC->value);
					FFmult_Novice = *reinterpret_cast<float*>(&NoviceFFChargeTimeMult_NPC->value);
					FFmult_Apprentice = *reinterpret_cast<float*>(&ApprenticeFFChargeTimeMult_NPC->value);
					FFmult_Adept = *reinterpret_cast<float*>(&AdeptFFChargeTimeMult_NPC->value);
					FFmult_Expert = *reinterpret_cast<float*>(&ExpertFFChargeTimeMult_NPC->value);
					FFmult_Master = *reinterpret_cast<float*>(&MasterFFChargeTimeMult_NPC->value);
					FFmult_Alteration = *reinterpret_cast<float*>(&AlterationFFChargeTimeMult_NPC->value);
					FFmult_Conjuration = *reinterpret_cast<float*>(&ConjurationFFChargeTimeMult_NPC->value);
					FFmult_Destruction = *reinterpret_cast<float*>(&DestructionFFChargeTimeMult_NPC->value);
					FFmult_Illusion = *reinterpret_cast<float*>(&IllusionFFChargeTimeMult_NPC->value);
					FFmult_Restoration = *reinterpret_cast<float*>(&RestorationFFChargeTimeMult_NPC->value);
					FFmult_Ritual = *reinterpret_cast<float*>(&RitualFFChargeTimeMult_NPC->value);

					ConcMod_Main = *reinterpret_cast<float*>(&ConcChargeTimeMod_NPC->value);
					ConcMod_Novice = *reinterpret_cast<float*>(&NoviceConcChargeTimeMod_NPC->value);
					ConcMod_Apprentice = *reinterpret_cast<float*>(&ApprenticeConcChargeTimeMod_NPC->value);
					ConcMod_Adept = *reinterpret_cast<float*>(&AdeptConcChargeTimeMod_NPC->value);
					ConcMod_Expert = *reinterpret_cast<float*>(&ExpertConcChargeTimeMod_NPC->value);
					ConcMod_Master = *reinterpret_cast<float*>(&MasterConcChargeTimeMod_NPC->value);
					ConcMod_Alteration = *reinterpret_cast<float*>(&AlterationConcChargeTimeMod_NPC->value);
					ConcMod_Conjuration = *reinterpret_cast<float*>(&ConjurationConcChargeTimeMod_NPC->value);
					ConcMod_Destruction = *reinterpret_cast<float*>(&DestructionConcChargeTimeMod_NPC->value);
					ConcMod_Illusion = *reinterpret_cast<float*>(&IllusionConcChargeTimeMod_NPC->value);
					ConcMod_Restoration = *reinterpret_cast<float*>(&RestorationConcChargeTimeMod_NPC->value);
					ConcMod_Ritual = *reinterpret_cast<float*>(&RitualConcChargeTimeMod_NPC->value);

					FFoverride_Novice = *reinterpret_cast<float*>(&NoviceDefaultFFOverride_NPC->value);
					FFoverride_Apprentice = *reinterpret_cast<float*>(&ApprenticeDefaultFFOverride_NPC->value);
					FFoverride_Adept = *reinterpret_cast<float*>(&AdeptDefaultFFOverride_NPC->value);
					FFoverride_Expert = *reinterpret_cast<float*>(&ExpertDefaultFFOverride_NPC->value);
					FFoverride_Master = *reinterpret_cast<float*>(&MasterDefaultFFOverride_NPC->value);
					FFoverride_Ritual = *reinterpret_cast<float*>(&RitualDefaultFFOverride_NPC->value);

					ConcOverride_Novice = *reinterpret_cast<float*>(&NoviceDefaultConcOverride_NPC->value);
					ConcOverride_Apprentice = *reinterpret_cast<float*>(&ApprenticeDefaultConcOverride_NPC->value);
					ConcOverride_Adept = *reinterpret_cast<float*>(&AdeptDefaultConcOverride_NPC->value);
					ConcOverride_Expert = *reinterpret_cast<float*>(&ExpertDefaultConcOverride_NPC->value);
					ConcOverride_Master = *reinterpret_cast<float*>(&MasterDefaultConcOverride_NPC->value);
					ConcOverride_Ritual = *reinterpret_cast<float*>(&RitualDefaultConcOverride_NPC->value);

					FFoverride_Toggle_Novice = *reinterpret_cast<float*>(&NoviceDefaultFFOverrideToggle_NPC->value);
					FFoverride_Toggle_Apprentice = *reinterpret_cast<float*>(&ApprenticeDefaultFFOverrideToggle_NPC->value);
					FFoverride_Toggle_Adept = *reinterpret_cast<float*>(&AdeptDefaultFFOverrideToggle_NPC->value);
					FFoverride_Toggle_Expert = *reinterpret_cast<float*>(&ExpertDefaultFFOverrideToggle_NPC->value);
					FFoverride_Toggle_Master = *reinterpret_cast<float*>(&MasterDefaultFFOverrideToggle_NPC->value);
					FFoverride_Toggle_Ritual = *reinterpret_cast<float*>(&RitualDefaultFFOverrideToggle_NPC->value);

					ConcOverride_Toggle_Novice = *reinterpret_cast<float*>(&NoviceDefaultConcOverrideToggle_NPC->value);
					ConcOverride_Toggle_Apprentice = *reinterpret_cast<float*>(&ApprenticeDefaultConcOverrideToggle_NPC->value);
					ConcOverride_Toggle_Adept = *reinterpret_cast<float*>(&AdeptDefaultConcOverrideToggle_NPC->value);
					ConcOverride_Toggle_Expert = *reinterpret_cast<float*>(&ExpertDefaultConcOverrideToggle_NPC->value);
					ConcOverride_Toggle_Master = *reinterpret_cast<float*>(&MasterDefaultConcOverrideToggle_NPC->value);
					ConcOverride_Toggle_Ritual = *reinterpret_cast<float*>(&RitualDefaultConcOverrideToggle_NPC->value);
				}
				switch(school) {
				case RE::ActorValue::kAlteration:
					FFmult_School = FFmult_Alteration;
					ConcMod_School = ConcMod_Alteration;

					//logger::info("School was Alteration");
					break;
				case RE::ActorValue::kConjuration:
					FFmult_School = FFmult_Conjuration;
					ConcMod_School = ConcMod_Conjuration;

					//logger::info("School was Conjuration");
					break;
				case RE::ActorValue::kDestruction:
					FFmult_School = FFmult_Destruction;
					ConcMod_School = ConcMod_Destruction;

					//logger::info("School was Destruction");
					break;
				case RE::ActorValue::kIllusion:
					FFmult_School = FFmult_Illusion;
					ConcMod_School = ConcMod_Illusion;

					//logger::info("School was Illusion");
					break;
				case RE::ActorValue::kRestoration:
					FFmult_School = FFmult_Restoration;
					ConcMod_School = ConcMod_Restoration;

					//logger::info("School was Restoration");
					break;
				}
				switch (difficulty) {
				case 0:
					FFmult_Difficulty = FFmult_Novice;
					ConcMod_Difficulty = ConcMod_Novice;
					FFoverride_Difficulty = FFoverride_Novice;
					ConcOverride_Difficulty = ConcOverride_Novice;
					FFoverride_Toggle_Difficulty = FFoverride_Toggle_Novice;
					ConcOverride_Toggle_Difficulty = ConcOverride_Toggle_Novice;
					//logger::info("Difficulty was Novice");
					break;
				case 25:
					FFmult_Difficulty = FFmult_Apprentice;
					ConcMod_Difficulty = ConcMod_Apprentice;
					FFoverride_Difficulty = FFoverride_Apprentice;
					ConcOverride_Difficulty = ConcOverride_Apprentice;
					FFoverride_Toggle_Difficulty = FFoverride_Toggle_Apprentice;
					ConcOverride_Toggle_Difficulty = ConcOverride_Toggle_Apprentice;
					//logger::info("Difficulty was Apprentice");
					break;
				case 50:
					FFmult_Difficulty = FFmult_Adept;
					ConcMod_Difficulty = ConcMod_Adept;
					FFoverride_Difficulty = FFoverride_Adept;
					ConcOverride_Difficulty = ConcOverride_Adept;
					FFoverride_Toggle_Difficulty = FFoverride_Toggle_Adept;
					ConcOverride_Toggle_Difficulty = ConcOverride_Toggle_Adept;
					//logger::info("Difficulty was Adept");
					break;
				case 75:
					FFmult_Difficulty = FFmult_Expert;
					ConcMod_Difficulty = ConcMod_Expert;
					FFoverride_Difficulty = FFoverride_Expert;
					ConcOverride_Difficulty = ConcOverride_Expert;
					FFoverride_Toggle_Difficulty = FFoverride_Toggle_Expert;
					ConcOverride_Toggle_Difficulty = ConcOverride_Toggle_Expert;
					//logger::info("Difficulty was Expert");
					break;
				case 90:
					FFmult_Difficulty = FFmult_Expert;
					ConcMod_Difficulty = ConcMod_Expert;
					FFoverride_Difficulty = FFoverride_Expert;
					ConcOverride_Difficulty = ConcOverride_Expert;
					FFoverride_Toggle_Difficulty = FFoverride_Toggle_Expert;
					ConcOverride_Toggle_Difficulty = ConcOverride_Toggle_Expert;
					//logger::info("Difficulty was Expert");
					break;
				case 100:
					FFmult_Difficulty = FFmult_Master;
					ConcMod_Difficulty = ConcMod_Master;
					FFoverride_Difficulty = FFoverride_Master;
					ConcOverride_Difficulty = ConcOverride_Master;
					FFoverride_Toggle_Difficulty = FFoverride_Toggle_Master;
					ConcOverride_Toggle_Difficulty = ConcOverride_Toggle_Master;
					//logger::info("Difficulty was Master");
					break;
				}
				if (debug == 1) {
					logger::info("For the spell: {}", spell->fullName.c_str());
					logger::info("The casting type is: {}", *reinterpret_cast<int*>(&casting));
					logger::info("The original charge time was: {}", time);
				}
				//time = GetChargeTimeVHook::func(spell);
				
				if (casting == RE::MagicSystem::CastingType::kFireAndForget) {
					if (FFoverride_Toggle_Difficulty == 1) {
						time = FFoverride_Difficulty;
					}
					if (ritual) {
						FFmult_Difficulty = FFmult_Ritual;
						if (FFoverride_Toggle_Ritual == 1) {
							time = FFoverride_Ritual;
						}
					}

					auto newTime = time * FFmult_Main * FFmult_School * FFmult_Difficulty;
					if (debug == 1) {
						logger::info("The new charge time is: {}", newTime);
					}
					return newTime;
				}
				if (casting == RE::MagicSystem::CastingType::kConcentration) {
					if (ConcOverride_Toggle_Difficulty == 1) {
						time = ConcOverride_Difficulty;
					}
					if (ritual) {
						ConcMod_Difficulty = ConcMod_Ritual;
						if (ConcOverride_Toggle_Ritual == 1) {
							time = ConcOverride_Ritual;
						}
					}

					auto newTime = time + ConcMod_Main + ConcMod_School + ConcMod_Difficulty;
					if (debug == 1) {
						logger::info("The new charge time is: {}", newTime);
					}
					return newTime;
				}
		}else {
			if (debug == 1) {
				logger::info("It wasn't conc or ff.");
			}
		}
		return time;
	}

	// Install our hook at the specified address
	void GetChargeTimeVHook::Install()
	{
		stl::write_vfunc<RE::SpellItem, GetChargeTimeVHook>();
		stl::write_vfunc<RE::ScrollItem, GetChargeTimeVHook>();
		logger::info("GetChargeTimeVHook hook set!");
	}

	void GetChargeTimeVHook::_UpdateGlobals()
	{
		ShowMsgs = RE::TESForm::LookupByEditorID("_CC_GlobalShowMsgs")->As<RE::TESGlobal>();
		
		//Player Globals
		FFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalFFChargeTimeMult")->As<RE::TESGlobal>();
		ConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalConcChargeTimeMod")->As<RE::TESGlobal>();

		RitualFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalRitualFFChargeTimeMult")->As<RE::TESGlobal>();
		RitualConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalRitualConcChargeTimeMod")->As<RE::TESGlobal>();
		RitualDefaultFFOverride = RE::TESForm::LookupByEditorID("_CC_GlobalRitualDefaultFFOverride")->As<RE::TESGlobal>();
		RitualDefaultConcOverride = RE::TESForm::LookupByEditorID("_CC_GlobalRitualDefaultConcOverride")->As<RE::TESGlobal>();
		RitualDefaultFFOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalRitualDefaultFFOverrideToggle")->As<RE::TESGlobal>();
		RitualDefaultConcOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalRitualDefaultConcOverrideToggle")->As<RE::TESGlobal>();

		AlterationFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalAlterationFFChargeTimeMult")->As<RE::TESGlobal>();
		AlterationConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalAlterationConcChargeTimeMod")->As<RE::TESGlobal>();
		DestructionFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalDestructionFFChargeTimeMult")->As<RE::TESGlobal>();
		DestructionConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalDestructionConcChargeTimeMod")->As<RE::TESGlobal>();
		ConjurationFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalConjurationFFChargeTimeMult")->As<RE::TESGlobal>();
		ConjurationConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalConjurationConcChargeTimeMod")->As<RE::TESGlobal>();
		IllusionFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalIllusionFFChargeTimeMult")->As<RE::TESGlobal>();
		IllusionConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalIllusionConcChargeTimeMod")->As<RE::TESGlobal>();
		RestorationFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalRestorationFFChargeTimeMult")->As<RE::TESGlobal>();
		RestorationConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalRestorationConcChargeTimeMod")->As<RE::TESGlobal>();

		NoviceFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceFFChargeTimeMult")->As<RE::TESGlobal>();
		NoviceConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceConcChargeTimeMod")->As<RE::TESGlobal>();
		ApprenticeFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeFFChargeTimeMult")->As<RE::TESGlobal>();
		ApprenticeConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeConcChargeTimeMod")->As<RE::TESGlobal>();
		AdeptFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptFFChargeTimeMult")->As<RE::TESGlobal>();
		AdeptConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptConcChargeTimeMod")->As<RE::TESGlobal>();
		ExpertFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalExpertFFChargeTimeMult")->As<RE::TESGlobal>();
		ExpertConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalExpertConcChargeTimeMod")->As<RE::TESGlobal>();
		MasterFFChargeTimeMult = RE::TESForm::LookupByEditorID("_CC_GlobalMasterFFChargeTimeMult")->As<RE::TESGlobal>();
		MasterConcChargeTimeMod = RE::TESForm::LookupByEditorID("_CC_GlobalMasterConcChargeTimeMod")->As<RE::TESGlobal>();

		NoviceDefaultFFOverride = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceDefaultFFOverride")->As<RE::TESGlobal>();
		ApprenticeDefaultFFOverride = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeDefaultFFOverride")->As<RE::TESGlobal>();
		AdeptDefaultFFOverride = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptDefaultFFOverride")->As<RE::TESGlobal>();
		ExpertDefaultFFOverride = RE::TESForm::LookupByEditorID("_CC_GlobalExpertDefaultFFOverride")->As<RE::TESGlobal>();
		MasterDefaultFFOverride = RE::TESForm::LookupByEditorID("_CC_GlobalMasterDefaultFFOverride")->As<RE::TESGlobal>();

		NoviceDefaultFFOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceDefaultFFOverrideToggle")->As<RE::TESGlobal>();
		ApprenticeDefaultFFOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeDefaultFFOverrideToggle")->As<RE::TESGlobal>();
		AdeptDefaultFFOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptDefaultFFOverrideToggle")->As<RE::TESGlobal>();
		ExpertDefaultFFOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalExpertDefaultFFOverrideToggle")->As<RE::TESGlobal>();
		MasterDefaultFFOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalMasterDefaultFFOverrideToggle")->As<RE::TESGlobal>();

		NoviceDefaultConcOverride = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceDefaultConcOverride")->As<RE::TESGlobal>();
		ApprenticeDefaultConcOverride = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeDefaultConcOverride")->As<RE::TESGlobal>();
		AdeptDefaultConcOverride = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptDefaultConcOverride")->As<RE::TESGlobal>();
		ExpertDefaultConcOverride = RE::TESForm::LookupByEditorID("_CC_GlobalExpertDefaultConcOverride")->As<RE::TESGlobal>();
		MasterDefaultConcOverride = RE::TESForm::LookupByEditorID("_CC_GlobalMasterDefaultConcOverride")->As<RE::TESGlobal>();

		NoviceDefaultConcOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceDefaultConcOverrideToggle")->As<RE::TESGlobal>();
		ApprenticeDefaultConcOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeDefaultConcOverrideToggle")->As<RE::TESGlobal>();
		AdeptDefaultConcOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptDefaultConcOverrideToggle")->As<RE::TESGlobal>();
		ExpertDefaultConcOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalExpertDefaultConcOverrideToggle")->As<RE::TESGlobal>();
		MasterDefaultConcOverrideToggle = RE::TESForm::LookupByEditorID("_CC_GlobalMasterDefaultConcOverrideToggle")->As<RE::TESGlobal>();

		//NPC Globals
		FFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		ConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalConcChargeTimeMod_NPC")->As<RE::TESGlobal>();

		RitualFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalRitualFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		RitualConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalRitualConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		RitualDefaultFFOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalRitualDefaultFFOverride_NPC")->As<RE::TESGlobal>();
		RitualDefaultConcOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalRitualDefaultConcOverride_NPC")->As<RE::TESGlobal>();
		RitualDefaultFFOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalRitualDefaultFFOverrideToggle_NPC")->As<RE::TESGlobal>();
		RitualDefaultConcOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalRitualDefaultConcOverrideToggle_NPC")->As<RE::TESGlobal>();

		AlterationFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalAlterationFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		AlterationConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalAlterationConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		DestructionFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalDestructionFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		DestructionConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalDestructionConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		ConjurationFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalConjurationFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		ConjurationConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalConjurationConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		IllusionFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalIllusionFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		IllusionConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalIllusionConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		RestorationFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalRestorationFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		RestorationConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalRestorationConcChargeTimeMod_NPC")->As<RE::TESGlobal>();

		NoviceFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		NoviceConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		ApprenticeFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		ApprenticeConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		AdeptFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		AdeptConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		ExpertFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalExpertFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		ExpertConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalExpertConcChargeTimeMod_NPC")->As<RE::TESGlobal>();
		MasterFFChargeTimeMult_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalMasterFFChargeTimeMult_NPC")->As<RE::TESGlobal>();
		MasterConcChargeTimeMod_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalMasterConcChargeTimeMod_NPC")->As<RE::TESGlobal>();

		NoviceDefaultFFOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceDefaultFFOverride_NPC")->As<RE::TESGlobal>();
		ApprenticeDefaultFFOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeDefaultFFOverride_NPC")->As<RE::TESGlobal>();
		AdeptDefaultFFOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptDefaultFFOverride_NPC")->As<RE::TESGlobal>();
		ExpertDefaultFFOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalExpertDefaultFFOverride_NPC")->As<RE::TESGlobal>();
		MasterDefaultFFOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalMasterDefaultFFOverride_NPC")->As<RE::TESGlobal>();

		NoviceDefaultFFOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceDefaultFFOverrideToggle_NPC")->As<RE::TESGlobal>();
		ApprenticeDefaultFFOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeDefaultFFOverrideToggle_NPC")->As<RE::TESGlobal>();
		AdeptDefaultFFOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptDefaultFFOverrideToggle_NPC")->As<RE::TESGlobal>();
		ExpertDefaultFFOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalExpertDefaultFFOverrideToggle_NPC")->As<RE::TESGlobal>();
		MasterDefaultFFOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalMasterDefaultFFOverrideToggle_NPC")->As<RE::TESGlobal>();

		NoviceDefaultConcOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceDefaultConcOverride_NPC")->As<RE::TESGlobal>();
		ApprenticeDefaultConcOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeDefaultConcOverride_NPC")->As<RE::TESGlobal>();
		AdeptDefaultConcOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptDefaultConcOverride_NPC")->As<RE::TESGlobal>();
		ExpertDefaultConcOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalExpertDefaultConcOverride_NPC")->As<RE::TESGlobal>();
		MasterDefaultConcOverride_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalMasterDefaultConcOverride_NPC")->As<RE::TESGlobal>();

		NoviceDefaultConcOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalNoviceDefaultConcOverrideToggle_NPC")->As<RE::TESGlobal>();
		ApprenticeDefaultConcOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalApprenticeDefaultConcOverrideToggle_NPC")->As<RE::TESGlobal>();
		AdeptDefaultConcOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalAdeptDefaultConcOverrideToggle_NPC")->As<RE::TESGlobal>();
		ExpertDefaultConcOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalExpertDefaultConcOverrideToggle_NPC")->As<RE::TESGlobal>();
		MasterDefaultConcOverrideToggle_NPC = RE::TESForm::LookupByEditorID("_CC_GlobalMasterDefaultConcOverrideToggle_NPC")->As<RE::TESGlobal>();

		logger::info("Retrieved global variables from .esp");
	}

	float GetChargeTime(RE::StaticFunctionTag*, RE::SpellItem* spell) {
		float hookedTime = ControlledCasting::GetChargeTimeVHook::thunk(spell);
		logger::info("Returning charge time ({}) called via papyrus", hookedTime);
		return hookedTime;
	}

	static void AddMultiplier(RE::StaticFunctionTag*, RE::TESGlobal* global)
	{

	}
	static void AddModifier(RE::StaticFunctionTag*, RE::TESGlobal* global)
	{
	
	}

	bool RegisterFuncs(RE::BSScript::IVirtualMachine* registry)
	{
		registry->RegisterFunction("GetChargeTime", "_CC_MCM", ControlledCasting::GetChargeTime, registry);

		return true;
	}

}


