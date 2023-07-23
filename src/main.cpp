#include "PCH.h"
#include "logger.h"
#include "skse/API.h"
#include "skse/SKSE.h"
#include "skse/Version.h"
#include <ShlObj.h>
#include "ControlledCasting.h"
#include "config.h"


using namespace RE::BSScript;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace
{
	/**
	 * \brief Sink to capture SKSE events.
	 * \param event - Submitted event.
	 */
	void OnEvent(SKSE::MessagingInterface::Message* event)
	{
		// After all the ESM/ESL/ESP plugins are loaded.
		if (event->type == SKSE::MessagingInterface::kDataLoaded) {
			logger::info("Data Loaded");
			auto* data = RE::TESDataHandler::GetSingleton();

			auto& spells = data->GetFormArray(RE::FormType::Spell);
			auto& npcs = data->GetFormArray(RE::FormType::NPC);

			for (auto*& form : spells) {
				if (auto* spell = static_cast<RE::SpellItem*>(form); spell) {
					//spell->data.flags.set(RE::SpellItem::SpellFlag::kCostOverride);
				}
			}

			ControlledCasting::GetChargeTimeVHook::_UpdateGlobals();
			config::GetFromJSON();
		}
		// When Skyrim starts, SKSE will begin by querying for SKSE plugins and then calling each plugin's SKSEPlugin_Load function.
		// Once all load functions are called this message is sent.
		else if (event->type == SKSE::MessagingInterface::kPostLoad) {
			logger::info("Post Load");
		}
		// The user has started a new game by selecting New Game at the main menu.
		else if (event->type == SKSE::MessagingInterface::kNewGame) {
			logger::info("New Game");
		}
		// The user has loaded a saved game.
		else if (event->type == SKSE::MessagingInterface::kPostLoadGame) {
			logger::info("Post Load Game");
			//ControlledCasting::GetChargeTimeVHook::_UpdateGlobals();
		}
		// Once all kPostLoad handlers have run, it will signal kPostPostLoad.
		else if (event->type == SKSE::MessagingInterface::kPostPostLoad) {
			logger::info("Post Post Load");
			
		}
	}

	void InitializePapyrus()
	{
		logger::info("Initializing Papyrus binding...");
		if (GetPapyrusInterface()->Register(ControlledCasting::RegisterFuncs)) {
			logger::info("Papyrus functions bound.");
		} else {
			logger::info("Failure to register Papyrus bindings.");
		}
	}

	/**
	 * \brief Returns the type of Skyrim that is running. Anniversary Edition, Special Edition, VR or Unknown.
	 * \return - Type of Skyrim that is running.
	 */
	static std::string_view GetRuntimeString()
	{
		switch (REL::Module::GetRuntime()) {
		case REL::Module::Runtime::AE:
			return "Anniversary Edition"sv;
		case REL::Module::Runtime::SE:
			return "Special Edition"sv;
		case REL::Module::Runtime::VR:
			return "VR"sv;
		default:;
		}
		return "Unknown"sv;
	}
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName(Version::PROJECT);
	v.AuthorName("_kmatt_");
	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST_AE });
	v.UsesNoStructs(true);

	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();

	return true;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
	SKSE::Init(skse);
	SetupLog();

	SKSE::GetMessagingInterface()->RegisterListener(OnEvent);

	ControlledCasting::GetChargeTimeVHook::Install();

	InitializePapyrus();

	return true;
};
