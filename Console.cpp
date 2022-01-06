#include "Console.h"

static mtConsole g_console;
mtConsole *console = &g_console;

// [SECTION] CVar

struct mtCVar
{
	virtual void SetDefault() = 0;
	virtual bool Set( const mtStr &src ) = 0;
	virtual mtStr GetDefault() = 0;
	virtual mtStr GetValue() = 0;
	virtual mtStr GetMin() = 0;
	virtual mtStr GetMax() = 0;
};

struct mtCVar_BOOL : public mtCVar
{
	bool *const src;
	const bool def;

	mtCVar_BOOL( bool *src, bool def ) : src( src ), def( def ) {};

	void SetDefault() override
	{
		*src = def;
	}

	bool Set( const mtStr &src ) override
	{
		if( !src.IsNumeric() )
			return false;

		bool converted_value = std::stoul( src.Data() );
		*this->src = converted_value;
		return true;
	}

	mtStr GetDefault() override
	{
		return std::to_string( def ).c_str();
	}

	mtStr GetValue() override
	{
		return std::to_string( *src ).c_str();
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

struct mtCVar_INT32 : public mtCVar
{
	int32 *const src;
	const int32 def;
	const int32 min;
	const int32 max;
	const bool minmax_defined;

	mtCVar_INT32( int32 *src, int32 def ) :
		src( src ),
		def( def ),
		min( 0 ),
		max( 0 ),
		minmax_defined( false )
	{
	};

	mtCVar_INT32( int32 *src, int32 def, int32 min, int32 max ) :
		src( src ),
		def( def ),
		min( min ),
		max( max ),
		minmax_defined( true )
	{
	};

	void SetDefault() override
	{
		*src = def;
	}

	bool Set( const mtStr &src ) override
	{
		if( !src.IsNumeric() )
			return false;

		int32 converted_value = (int32)std::stoi( src.Data() );

		if( minmax_defined )
		{
			if( converted_value < min )
				converted_value = min;
			if( converted_value > max )
				converted_value = max;
		}

		*this->src = converted_value;

		return true;
	}

	mtStr GetDefault() override
	{
		return std::to_string( def ).c_str();
	}

	mtStr GetValue() override
	{
		return std::to_string( *src ).c_str();
	}

	mtStr GetMin() override
	{
		if( !minmax_defined )
			return "not_defined";

		return std::to_string( min ).c_str();
	}

	mtStr GetMax() override
	{
		if( !minmax_defined )
			return "not_defined";

		return std::to_string( max ).c_str();
	}
};

struct mtCVar_UINT32 : public mtCVar
{
	uint32 *const src;
	const uint32 def;
	const uint32 min;
	const uint32 max;
	const bool minmax_defined;

	mtCVar_UINT32( uint32 *src, uint32 def ) :
		src( src ),
		def( def ),
		min( 0 ),
		max( 0 ),
		minmax_defined( false )
	{
	};

	mtCVar_UINT32( uint32 *src, uint32 def, uint32 min, uint32 max ) :
		src( src ),
		def( def ),
		min( min ),
		max( max ),
		minmax_defined( true )
	{
	};

	void SetDefault() override
	{
		*src = def;
	}

	bool Set( const mtStr &src ) override
	{
		if( !src.IsNumeric() )
			return false;

		uint32 converted_value = (uint32)std::stoul( src.Data() );

		if( minmax_defined )
		{
			if( converted_value < min )
				converted_value = min;
			if( converted_value > max )
				converted_value = max;
		}

		*this->src = converted_value;

		return true;
	}

	mtStr GetDefault() override
	{
		return std::to_string( def ).c_str();
	}

	mtStr GetValue() override
	{
		return std::to_string( *src ).c_str();
	}

	mtStr GetMin() override
	{
		if( !minmax_defined )
			return "not_defined";

		return std::to_string( min ).c_str();
	}

	mtStr GetMax() override
	{
		if( !minmax_defined )
			return "not_defined";

		return std::to_string( max ).c_str();
	}
};

struct mtCVar_FLOAT : public mtCVar
{
	float *const src;
	const float def;
	const float min;
	const float max;
	const bool minmax_defined;

	mtCVar_FLOAT( float *src, float def ) :
		src( src ),
		def( def ),
		min( 0 ),
		max( 0 ),
		minmax_defined( false )
	{
	};

	mtCVar_FLOAT( float *src, float def, float min, float max ) :
		src( src ),
		def( def ),
		min( min ),
		max( max ),
		minmax_defined( true )
	{
	};

	void SetDefault() override
	{
		*src = def;
	}

	bool Set( const mtStr &src ) override
	{
		if( !src.IsNumericFloat() )
			return false;

		float converted_value = std::stof( src.Data() );

		if( minmax_defined )
		{
			if( converted_value < min )
				converted_value = min;
			if( converted_value > max )
				converted_value = max;
		}

		*this->src = converted_value;

		return true;
	}

	mtStr GetDefault() override
	{
		return std::to_string( def ).c_str();
	}

	mtStr GetValue() override
	{
		return std::to_string( *src ).c_str();
	}

	mtStr GetMin() override
	{
		if( !minmax_defined )
			return "not_defined";

		return std::to_string( min ).c_str();
	}

	mtStr GetMax() override
	{
		if( !minmax_defined )
			return "not_defined";

		return std::to_string( max ).c_str();
	}
};

void mtConsole::Command( mtStr input )
{
	input.ToLower();

	mtStr command;
	mtStr value;

	bool space_found = false;

	for( int i = 0; i < input.Length(); i++ )
	{
		if( input[i] == ' ' )
			space_found = true;

		if( !space_found )
			command += input[i];
		else
			value += input[i];
	}

	// List all the possible cvar's.
	if( command.Find( "cvarlist" ) )
	{
		for( auto &e : registeredCVarMap )
		{
			mtCVar *cvar = e.second;
			console->Message( e.first + " [ Value: " + cvar->GetValue() + " ] [ Default: " + cvar->GetDefault() + " ] [ Min: " + cvar->GetMin() + " ] [ Max: " + cvar->GetMax() + " ]" );
		}
		return;
	}

	auto find = registeredCVarMap.find( command );

	if( find != registeredCVarMap.end() )
	{
		mtCVar *cvar = find->second;

		value.FindAndRemoveAll( " " );

		// If no value was given, then just print the command and all its related info.
		if( value.IsEmpty() )
		{
			console->Message( command + " [ Value: " + cvar->GetValue() + " ] [ Default: " + cvar->GetDefault() + " ] [ Min: " + cvar->GetMin() + " ] [ Max: " + cvar->GetMax() + " ]" );
			return;
		}

		// If value is "def", then set the default value of the cvar.
		if( value.Find( "def" ) )
		{
			cvar->SetDefault();
			console->Success( command + " is set to default value: " + cvar->GetValue() );
			return;
		}

		if( !cvar->Set( value ) )
		{
			console->Message( mtStr( "Unacceptable value: " ) + value );
			return;
		}

		console->Success( command + " is set to: " + cvar->GetValue() );
	}
	else
	{
		console->Message( mtStr( "Unknown command: " ) + command );
	}
}

void mtConsole::RegisterCVar_BOOL( mtStr command, bool *src, bool def )
{
	registeredCVarMap.emplace( command, new mtCVar_BOOL( src, def ) );
}

void mtConsole::RegisterCVar_INT32( mtStr command, int32 *src, int32 def )
{
	registeredCVarMap.emplace( command, new mtCVar_INT32( src, def ) );
}

void mtConsole::RegisterCVar_INT32( mtStr command, int32 *src, int32 def, int32 min, int32 max )
{
	registeredCVarMap.emplace( command, new mtCVar_INT32( src, def, min, max ) );
}

void mtConsole::RegisterCVar_UINT32( mtStr command, uint32 *src, uint32 def )
{
	registeredCVarMap.emplace( command, new mtCVar_UINT32( src, def ) );
}

void mtConsole::RegisterCVar_UINT32( mtStr command, uint32 *src, uint32 def, uint32 min, uint32 max )
{
	registeredCVarMap.emplace( command, new mtCVar_UINT32( src, def, min, max ) );
}

void mtConsole::RegisterCVar_FLOAT( mtStr command, float *src, float def )
{
	registeredCVarMap.emplace( command, new mtCVar_FLOAT( src, def ) );
}

void mtConsole::RegisterCVar_FLOAT( mtStr command, float *src, float def, float min, float max )
{
	registeredCVarMap.emplace( command, new mtCVar_FLOAT( src, def, min, max ) );
}

void mtConsole::Message( mtStr text )
{
	mtCMsg message;
	message.text = text;
	message.color_hex = 0xFFFFFFFF;
	messageList.push_back( message );
}

void mtConsole::Message( mtStr text, uint32 color_hex )
{
	mtCMsg message;
	message.text = text;
	message.color_hex = color_hex;
	messageList.push_back( message );
}

void mtConsole::Error( mtStr text )
{
	Message( text, mtColor::HEX_RED );
}

void mtConsole::Success( mtStr text )
{
	Message( text, mtColor::HEX_GREEN );
}

void mtConsole::Warning( mtStr text )
{
	Message( text, mtColor::HEX_YELLOW );
}

void mtConsole::Clear()
{
	messageList.clear();
}

void mtConsole::StoreCVarList()
{
	mtFileOut file( IOMode::plain, true );
	if( file.Open( L"Data\\cvar.mtcfg" ) )
	{
		for( auto &command : console->GetCVarMap() )
		{
			mtStr out;
			out = command.first + " " + command.second->GetValue() + "\n";
			file.Write( out.Data(), out.Length() );
		}
	}
}

void mtConsole::RestoreCVarList()
{
	mtFileIn file( IOMode::plain );
	if( file.Open( L"Data\\cvar.mtcfg" ) )
	{
		std::string dest;
		while( file.ReadLine( dest ) )
			console->Command( dest.c_str() );
	}
}

const std::vector<mtCMsg> &mtConsole::GetMessageList() const
{
	return messageList;
}

const CVarMap &mtConsole::GetCVarMap() const
{
	return registeredCVarMap;
}
