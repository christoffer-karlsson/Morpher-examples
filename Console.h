#pragma once

#include "../Graphics/Color.h"
#include "../Library/String.h"
#include "../System/File.h"
#include <unordered_map>
#include <vector>

#undef min
#undef max

struct mtCVAR;

struct mtCVARMessage
{
	mtStr text = mtStr(mtConfig::MAX_CONSOLE_MESSAGE_STRING);
	uint32 color_hex = 0xFFFFFFFF;
};

typedef std::unordered_map<mtStr, mtCVAR *> mtCVARMap;

class mtConsole
{
	public:

	mtConsole();
	// Check input from user.
	void Command(mtStr input);
	// Register a variable.
	void RegisterCVar_BOOL(const mtStr &command, bool *src, bool def);
	void RegisterCVar_INT32(const mtStr &command, int32 *src, int32 def);
	void RegisterCVar_INT32(const mtStr &command, int32 *src, int32 def, int32 min, int32 max);
	void RegisterCVar_UINT32(const mtStr &command, uint32 *src, uint32 def);
	void RegisterCVar_UINT32(const mtStr &command, uint32 *src, uint32 def, uint32 min, uint32 max);
	void RegisterCVar_FLOAT(const mtStr &command, float *src, float def);
	void RegisterCVar_FLOAT(const mtStr &command, float *src, float def, float min, float max);
	// Print console message.
	void Message(const mtStr &text, uint32 color_hex);
	void Message(const mtStr &text);
	void Success(const mtStr &text);
	void Warning(const mtStr &text);
	void Error(const mtStr &text);
	// Handle overflow of messages.
	void Overflow(const mtStr &text, uint32 color_hex);
	// Clear console messages.
	void Clear();
	// Write the CVar map to disk.
	void Save();
	// Load the CVar map from disk.
	void Load();

	size_t GetMessageCount() const;
	const mtCVARMap &GetCVarMap() const;
	const mtCVARMessage *Iteration(size_t &i) const;

	private:

	mtFile file;
	mtCVARMap cvar_map;
	mtList<mtCVARMessage *> list_message;
	mtAllocator<mtCVARMessage> alloc_message;
};

extern mtConsole *console;
