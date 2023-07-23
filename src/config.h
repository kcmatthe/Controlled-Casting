#pragma once

#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>
#include <cstdio>
#include "ControlledCasting.h"

namespace config
{
	
	bool GetFromJSON();
	
}

