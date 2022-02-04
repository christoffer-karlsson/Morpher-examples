#include "Console.h"

static mtConsole g_console;
mtConsole *console = &g_console;

struct mtCVAR
{
	virtual void SetDefault() = 0;
	virtual bool Set(const mtStr &src) = 0;
	virtual mtStr GetDefault() = 0;
	virtual mtStr GetValue() = 0;
	virtual mtStr GetMin() = 0;
	virtual mtStr GetMax() = 0;
};

struct mtCVar_BOOL : public mtCVAR
{
	bool *const src;
	const bool def;

	mtCVar_BOOL(bool *src, bool def) : src(src), def(def) {};

	void SetDefault() override
	{
		*src = def;
	}

	bool Set(const mtStr &src) override
	{
		if(!src.IsNumeric())
			return false;

		bool converted_value = std::stoul(src.Data());
		*this->src = converted_value;
		return true;
	}

	mtStr GetDefault() override
	{
		return std::to_string(def).c_str();
	}

	mtStr GetValue() override
	{
		return std::to_string(*src).c_str();
	}

	mtStr GetMin() override
	{
		return "false (0)";
	}

	mtStr GetMax() override
	{
		return "true (1)";
	}
};

struct mtCVar_INT32 : public mtCVAR
{
	int32 *const src;
	const int32 def;
	const int32 min;
	const int32 max;
	const bool minmax_defined;

	mtCVar_INT32(int32 *src, int32 def) :
		src(src),
		def(def),
		min(0),
		max(0),
		minmax_defined(false)
	{
	};

	mtCVar_INT32(int32 *src, int32 def, int32 min, int32 max) :
		src(src),
		def(def),
		min(min),
		max(max),
		minmax_defined(true)
	{
	};

	void SetDefault() override
	{
		*src = def;
	}

	bool Set(const mtStr &src) override
	{
		if(!src.IsNumeric())
			return false;

		int32 converted_value = (int32)std::stoi(src.Data());

		if(minmax_defined)
		{
			if(converted_value < min)
				converted_value = min;
			if(converted_value > max)
				converted_value = max;
		}

		*this->src = converted_value;

		return true;
	}

	mtStr GetDefault() override
	{
		return std::to_string(def).c_str();
	}

	mtStr GetValue() override
	{
		return std::to_string(*src).c_str();
	}

	mtStr GetMin() override
	{
		if(!minmax_defined)
			return "not_defined";

		return std::to_string(min).c_str();
	}

	mtStr GetMax() override
	{
		if(!minmax_defined)
			return "not_defined";

		return std::to_string(max).c_str();
	}
};

struct mtCVar_UINT32 : public mtCVAR
{
	uint32 *const src;
	const uint32 def;
	const uint32 min;
	const uint32 max;
	const bool minmax_defined;

	mtCVar_UINT32(uint32 *src, uint32 def) :
		src(src),
		def(def),
		min(0),
		max(0),
		minmax_defined(false)
	{
	};

	mtCVar_UINT32(uint32 *src, uint32 def, uint32 min, uint32 max) :
		src(src),
		def(def),
		min(min),
		max(max),
		minmax_defined(true)
	{
	};

	void SetDefault() override
	{
		*src = def;
	}

	bool Set(const mtStr &src) override
	{
		if(!src.IsNumeric())
			return false;

		uint32 converted_value = (uint32)std::stoul(src.Data());

		if(minmax_defined)
		{
			if(converted_value < min)
				converted_value = min;
			if(converted_value > max)
				converted_value = max;
		}

		*this->src = converted_value;

		return true;
	}

	mtStr GetDefault() override
	{
		return std::to_string(def).c_str();
	}

	mtStr GetValue() override
	{
		return std::to_string(*src).c_str();
	}

	mtStr GetMin() override
	{
		if(!minmax_defined)
			return "not_defined";

		return std::to_string(min).c_str();
	}

	mtStr GetMax() override
	{
		if(!minmax_defined)
			return "not_defined";

		return std::to_string(max).c_str();
	}
};

struct mtCVar_FLOAT : public mtCVAR
{
	float *const src;
	const float def;
	const float min;
	const float max;
	const bool minmax_defined;

	mtCVar_FLOAT(float *src, float def) :
		src(src),
		def(def),
		min(0),
		max(0),
		minmax_defined(false)
	{
	};

	mtCVar_FLOAT(float *src, float def, float min, float max) :
		src(src),
		def(def),
		min(min),
		max(max),
		minmax_defined(true)
	{
	};

	void SetDefault() override
	{
		*src = def;
	}

	bool Set(const mtStr &src) override
	{
		if(!src.IsNumericFloat())
			return false;

		float converted_value = std::stof(src.Data());

		if(minmax_defined)
		{
			if(converted_value < min)
				converted_value = min;
			if(converted_value > max)
				converted_value = max;
		}

		*this->src = converted_value;

		return true;
	}

	mtStr GetDefault() override
	{
		return std::to_string(def).c_str();
	}

	mtStr GetValue() override
	{
		return std::to_string(*src).c_str();
	}

	mtStr GetMin() override
	{
		if(!minmax_defined)
			return "not_defined";

		return std::to_string(min).c_str();
	}

	mtStr GetMax() override
	{
		if(!minmax_defined)
			return "not_defined";

		return std::to_string(max).c_str();
	}
};

mtConsole::mtConsole() :
	alloc_message(mtConfig::MAX_CONSOLE_MESSAGE_COUNT),
	list_message(mtConfig::MAX_CONSOLE_MESSAGE_COUNT)
{}

void mtConsole::Command(mtStr input)
{
	input.ToLower();

	mtStr command;
	mtStr value;

	bool space_found = false;

	for(int i = 0; i < input.Length(); i++)
	{
		if(input[i] == ' ')
			space_found = true;

		if(!space_found)
			command += input[i];
		else
			value += input[i];
	}

	// List all the possible cvar's.
	if(command.Find("cvarlist"))
	{
		for(auto &e : cvar_map)
		{
			mtCVAR *cvar = e.second;
			console->Message(e.first + " [ Value: " + cvar->GetValue() + " ] [ Default: " + cvar->GetDefault() + " ] [ Min: " + cvar->GetMin() + " ] [ Max: " + cvar->GetMax() + " ]");
		}
		return;
	}

	auto find = cvar_map.find(command);

	if(find != cvar_map.end())
	{
		mtCVAR *cvar = find->second;

		value.FindAndRemoveAll(" ");

		// If no value was given, then just print the command and all its related info.
		if(value.IsEmpty())
		{
			console->Message(command + " [ Value: " + cvar->GetValue() + " ] [ Default: " + cvar->GetDefault() + " ] [ Min: " + cvar->GetMin() + " ] [ Max: " + cvar->GetMax() + " ]");
			return;
		}

		// If value is "def", then set the default value of the cvar.
		if(value.Find("def"))
		{
			cvar->SetDefault();
			console->Success(command + " is set to default value: " + cvar->GetValue());
			return;
		}

		if(!cvar->Set(value))
		{
			console->Message(mtStr("Unacceptable value: ") + value);
			return;
		}

		console->Success(command + " is set to: " + cvar->GetValue());
	}
	else
	{
		console->Message(mtStr("Unknown command: ") + command);
	}
}

void mtConsole::RegisterCVar_BOOL(const mtStr &command, bool *src, bool def)
{
	cvar_map.emplace(command, new mtCVar_BOOL(src, def));
}

void mtConsole::RegisterCVar_INT32(const mtStr &command, int32 *src, int32 def)
{
	cvar_map.emplace(command, new mtCVar_INT32(src, def));
}

void mtConsole::RegisterCVar_INT32(const mtStr &command, int32 *src, int32 def, int32 min, int32 max)
{
	cvar_map.emplace(command, new mtCVar_INT32(src, def, min, max));
}

void mtConsole::RegisterCVar_UINT32(const mtStr &command, uint32 *src, uint32 def)
{
	cvar_map.emplace(command, new mtCVar_UINT32(src, def));
}

void mtConsole::RegisterCVar_UINT32(const mtStr &command, uint32 *src, uint32 def, uint32 min, uint32 max)
{
	cvar_map.emplace(command, new mtCVar_UINT32(src, def, min, max));
}

void mtConsole::RegisterCVar_FLOAT(const mtStr &command, float *src, float def)
{
	cvar_map.emplace(command, new mtCVar_FLOAT(src, def));
}

void mtConsole::RegisterCVar_FLOAT(const mtStr &command, float *src, float def, float min, float max)
{
	cvar_map.emplace(command, new mtCVar_FLOAT(src, def, min, max));
}

void mtConsole::Message(const mtStr &text, uint32 color_hex)
{
	if(list_message.Count() >= mtConfig::MAX_CONSOLE_MESSAGE_COUNT)
	{
		Overflow(text, color_hex);
		return;
	}

	mtCVARMessage *message = alloc_message.Get();
	message->text.Edit(text.Data());
	message->color_hex = color_hex;

	list_message.Append(message);
}

void mtConsole::Message(const mtStr &text)
{
	Message(text, 0xFFFFFFFF);
}

void mtConsole::Success(const mtStr &text)
{
	Message(text, mtColor::HEX_GREEN);
}

void mtConsole::Warning(const mtStr &text)
{
	Message(text, mtColor::HEX_YELLOW);
}

void mtConsole::Error(const mtStr &text)
{
	Message(text, mtColor::HEX_RED);
}

void mtConsole::Overflow(const mtStr &text, uint32 color_hex)
{
	// How many messages that should be saved before flushing.
	const int saved = 5;

	int begin = list_message.Count() - saved;

	mtCVARMessage temp[saved];

	int j = 0;
	for(int i = begin; i < list_message.Count(); i++)
		temp[j++] = *list_message[i];

	Clear();

	for(int j = 0; j < saved; j++)
		Message(temp[j].text, temp[j].color_hex);

	Message(text, color_hex);
}

void mtConsole::Clear()
{
	// Return all the pointers to the allocator.
	for(int i = 0; i < list_message.Count(); i++)
		alloc_message.Return(list_message[i]);

	list_message.Reset();
}

void mtConsole::Save()
{
	file.Open(mtConfig::FILE_PATH_CVAR, write | discard);

	for(auto &command : console->GetCVarMap())
	{
		mtStr out;
		out = command.first + " " + command.second->GetValue() + "\n";
		file.Write(out.Data(), out.Length());
	}

	file.Close();
}

void mtConsole::Load()
{
	file.Open(mtConfig::FILE_PATH_CVAR, read);

	mtStr dest;
	while(!file.CheckEOF())
	{
		if(file.ReadPeek() == EOF)
			break;

		dest = file.ReadLine();
		console->Command(dest);

		if(file.CheckFailBit())
			break;
	}

	file.Close();
}

size_t mtConsole::GetMessageCount() const
{
	return list_message.Count();
}

const mtCVARMap &mtConsole::GetCVarMap() const
{
	return cvar_map;
}

const mtCVARMessage *mtConsole::Iteration(size_t &i) const
{
	mtCVARMessage *result = nullptr;

	if(i < list_message.Count())
	{
		result = list_message[i];
		i++;
	}

	return result;
}
