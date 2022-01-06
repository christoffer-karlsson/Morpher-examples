#pragma once

#include "../System/File.h"
#include "../Graphics/Color.h"
#include "../MorphtanLib/Str.h"
#include <unordered_map>
#include <vector>

#undef min
#undef max

struct mtCMsg
{
	mtStr text;
	uint32 color_hex;
};

struct mtCVar;

typedef std::unordered_map<mtStr, mtCVar *> CVarMap;
typedef std::vector<mtCMsg> MsgList;

class mtConsole
{
	public:

	// Check input from user.
	void Command( mtStr input );
	// Register a variable.
	void RegisterCVar_BOOL( mtStr command, bool *src, bool def );
	void RegisterCVar_INT32( mtStr command, int32 *src, int32 def );
	void RegisterCVar_INT32( mtStr command, int32 *src, int32 def, int32 min, int32 max );
	void RegisterCVar_UINT32( mtStr command, uint32 *src, uint32 def );
	void RegisterCVar_UINT32( mtStr command, uint32 *src, uint32 def, uint32 min, uint32 max );
	void RegisterCVar_FLOAT( mtStr command, float *src, float def );
	void RegisterCVar_FLOAT( mtStr command, float *src, float def, float min, float max );
	// Print console message.
	void Message( mtStr text );
	void Message( mtStr text, uint32 color_hex );
	void Error( mtStr text );
	void Success( mtStr text );
	void Warning( mtStr text );
	// Clear console messages.
	void Clear();
	// Write the CVar map to disk.
	void StoreCVarList();
	// Load the CVar map from disk.
	void RestoreCVarList();
	const MsgList &GetMessageList() const;
	const CVarMap &GetCVarMap() const;

	private:

	MsgList messageList;
	CVarMap registeredCVarMap;
};

extern mtConsole *console;