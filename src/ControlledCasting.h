#pragma once

#include "PCH.h"


namespace ControlledCasting {

	struct GetChargeTimeVHook
	{
	public:
		static float thunk(RE::SpellItem* a_self);
		
		static inline std::uint32_t idx = 100;

		static inline REL::Relocation<decltype(thunk)> func;

		static void Install();

		static void _UpdateGlobals();

		
	};

	struct Override
	{
	public:
		RE::SpellItem* spell;
		bool excluded = false;
		bool override;
		float value;

		Override(RE::SpellItem* f, bool e, bool o, float val)
		{
			spell = f;
			excluded = e;
			override = o;
			value = val;
		}
	};	

	//Use globals? Convert even my existing mults and modifiers to use a similar system?
	struct Multiplier
	{
	public:
		std::string name;
		float value;

		Multiplier(std::string s, float v) {
			name = s;
			value = v;
		}
	};

	struct Modifier
	{
	public:
		std::string name;
		float value;

		Modifier(std::string s, float v)
		{
			name = s;
			value = v;
		}
	};

	float GetChargeTime(RE::StaticFunctionTag*, RE::SpellItem* spell);
	static void AddMultiplier(RE::StaticFunctionTag*, RE::TESGlobal* global);
	static void AddModifier(RE::StaticFunctionTag*, RE::TESGlobal* global);

	bool RegisterFuncs(RE::BSScript::IVirtualMachine* registry);
	
}


