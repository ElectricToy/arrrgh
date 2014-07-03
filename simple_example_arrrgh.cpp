#include "arrrgh.hpp"

int main( int argc, const char* argv[] )
{
	arrrgh::parser parser( "arrrghsample", "Parses a mix of many argument types and combinations." );
	
	const auto& useAscii = parser.add< bool >( "ascii",
											  "Use ASCII instead of that UNICORN thing or whatever it is.",
											  'a',
											  arrrgh::Optional,
											  true /* defaults to true */ );
	const auto& runFast = parser.add< bool >( "fast", "Should this program run fast?", 'f' );
	const auto& doCalibrate = parser.add< bool >( "calibrate", "Calibrate sensors." /* no short-form */ );
	const auto& kindaOdd = parser.add< bool >( "", "No long-form argument for this one.", 'o' );
	const auto& mass = parser.add< float >( "mass", "The mass of the thing.", 'm', arrrgh::Optional, 3.141f );
	const auto& mana = parser.add< float >( "mana", "The mana of the thing.", 'M' );
	const auto& height = parser.add< int >( "height", "The height of the thing.", 'h', arrrgh::Required );
	const auto& name = parser.add< std::string >( "name", "The name of the wind.", 's', arrrgh::Required );
	
	// Unleash the hounds.
	//
	try
	{
		// Example command line:
		//	arrrghsample --height=16.25 -fo unlabeled --name="Absolom, Absolom" -h=8 -- --weirdly-unlabeled
		parser.parse( argc, argv );
	}
	catch( const std::exception& e )
	{
		std::cerr << "Error parsing arguments: " << e.what() << std::endl;
		parser.show_usage( std::cerr );
		exit( 1 );
	}
	
	parser.show_usage( std::cerr );

	// Get argument values.
	//
	try
	{
		std::cout << std::boolalpha;	// So that bool values come through as "true/false" rather than "1/0".
		std::cout << "useAscii=" << useAscii.value() << std::endl;
		std::cout << "runFast=" << runFast.value() << std::endl;
		std::cout << "doCalibrate=" << doCalibrate.value() << std::endl;
		std::cout << "kindaOdd=" << kindaOdd.value() << std::endl;
		std::cout << "mass=" << mass.value() << std::endl;
		std::cout << "mana=" << mana.value() << std::endl;
		std::cout << "height=" << height.value() << std::endl;
		std::cout << "name=" << name.value() << std::endl;
		
		// What about unlabeled arguments?
		//
		// Notice that "--weirdly-unlabeled" looks like a switch but comes through nicely as unlabeled.
		// That's because the example command line indicated "--" before it, which ends switch parsing
		// (everything afterward becomes unlabeled).
		//
		std::cout << "Unlabeled:\n";
		parser.each_unlabeled_argument( []( const std::string& arg ) { std::cout << "\t" << arg << std::endl; } );
	}
	catch( const std::exception& e )
	{
		std::cerr << "Error reading argument values: " << e.what() << std::endl;
	}
	
	return 0;
}