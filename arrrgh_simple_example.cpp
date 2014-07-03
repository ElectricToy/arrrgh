#include "arrrgh.hpp"

int main( int argc, const char* argv[] )
{
	arrrgh::parser parser( "enchilada", "The whole enchilada: a mix of many argument types and situations." );
	
	const auto& useAscii = program.parser().add< bool >( "ascii",
														"Use ASCII instead of that UNICORN thing or whatever it is.",
														'a',
														arrrgh::Optional,
														true /* defaults to true */ );
	const auto& runFast = program.parser().add< bool >( "fast", "Should this program run fast?", 'b' );
	const auto& doCalibrate = program.parser().add< bool >( "calibrate", "Calibrate sensors.", 'c' );
	const auto& kindaOdd = program.parser().add< bool >( "", "No long-form argument for this one.", 'd' );
	const auto& mass = program.parser().add< float >( "mass", "The mass of the thing.", 'm', arrrgh::Optional, 3.141f );
	const auto& mana = program.parser().add< float >( "mana", "The mana of the thing.", 'n' );
	const auto& height = program.parser().add< int >( "height", "The height of the thing.", 'h', arrrgh::Required );
	const auto& name = program.parser().add< std::string >( "name", "The name of the wind.", 's', arrrgh::Required );
	
	// Unleash the hounds.
	//
	try
	{
		// Example command line:
		//	enchilada --height=16.25 -bd unlabeled --name=\"Absolom, Absolom\" -h=8 -- --weirdly-unlabeled
		parser.parse( argc, argv );
	}
	catch( const std::exception& e )
	{
		std::cerr << "Error parsing arguments: " << e.what() << std::endl;
		parser.show_usage( std::cerr );
		exit( 1 );
	}
	
	// Get argument values.
	//
	try
	{
		// Example access:
		std::cout << "height=" << height.value() << std::endl;
	}
	catch( const std::exception& e )
	{
		std::cerr << "Error reading argument values: " << e.what() << std::endl;
	}
	
	return 0;
}
