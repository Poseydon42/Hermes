#include <iostream>
#include <fstream>
#include <string>

#include "Core/Misc/ArgsParser.h"
#include "FileProcessor.h"
#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes::Tools
{	
	void ShowHelp()
	{
		const auto* HelpMessage = R"(
		Usage:
			meshtoasset [OPTIONS] file
		Options:
			--flip, -f: flip order of vertices in triangles
			--help, -h: display this help message
		)";
		std::cout << HelpMessage << std::endl;
	}

	int Main(int ArgCount, char** ArgValues)
	{
		if (ArgCount < 2)
		{
			std::cerr << "You need to specify input file; use --help for help" << std::endl;
			return 1;
		}

		bool FlipVertexOrder = false;
		bool ShowHelpOption = false;
		String FileName;

		ArgsParser Parser;
		Parser.AddOption("flip", 'f', &FlipVertexOrder);
		Parser.AddOption("help", 'h', &ShowHelpOption);
		Parser.AddPositional(true, &FileName);

		if (!Parser.Parse(ArgCount - 1, const_cast<const char**>(ArgValues + 1)))
		{
			std::cerr << "Bad arguments; use --help for help" << std::endl;
			return 1;
		}

		if (ShowHelpOption)
		{
			ShowHelp();
			return 0;
		}

		FileProcessor Processor(FileName, FlipVertexOrder);

		return Processor.Run();
	}
}

/*
 * Usage:
 * meshtoasset <input file> [options]
 * Possible options:
 *  --obj: override file extension and parse it as OBJ file
 *	--flip, -f: flip triangle order(clockwise vs counterclockwise)
 * Currently supported mesh formats:
 *  - OBJ
 */
int main(int ArgCount, char** ArgValues)
{
	return Hermes::Tools::Main(ArgCount, ArgValues);
}
