Arrrgh
======

Jeff Wofford | July 2014 | e:  biz@jeffwofford.com | w:  http://www.jeffwofford.com

**arrrgh** is a fast, small, simple, powerful, single-header library for parsing command line arguments in C++ using more-or-less POSIX parsing rules. It is written using modern C++11 paradigms so it's safe, fast, and tends to invite clean, small, to-the-point code.

There are lots of other command line parsers out there. Yet every time I wrote a new command line program, I ended up rolling my own underpowered and error-prone argument parsing. I like [TCLAP](http://tclap.sourceforge.net/) but it's a full library with makefiles, lots of source and headers, and the rest. Likewise for getopt, boost, and others: the command line parsing code can end up vastly outweighing the program it's meant to serve. Plus, integrating headers and libraries into C/C++ programs is never a lot of fun, especially when porting to different IDEs and compilers. [The Lean Mean Option Parser](http://optionparser.sourceforge.net/index.html) is nice and small but I didn't prefer its style. You can disagree.

I wrote **arrrgh** to escape the madness. It's a single file. You copy it into your project. You include it. You set up the arguments you want to support. It does the rest.

# To Use

1.	Include the header:

		#include "arrrgh.hpp"

	There's no .lib or .so or .cpp or anything else to muck around with. It's all here.

2.	Create a parser object and give it your program name and description (for the "usage" output):

		arrrgh::parser parser( "<my-prog>", "<description>" );

3.	Add your arguments, templated on the desired type. Example:

		const auto& myArgument = parser.add< float >( ... );

4.	In the add() function you'll indicate the long-form ("--example") and short-form ("-e") switches for this 
	argument. You can nullify one or the other if you want.
	You should specify a description (the second argument).
	You may also specify whether the argument is optional or required, and what the default value is (in case 
	the user doesn't supply it):

		... parser.add< float >(
				"<long-form>",			// Use "example" if you want the user to say "--example", 
										// or leave blank for no long-form.
				"<description>",		// For "usage" output.
				'<short-form>', 		// A character. Use '\0' for no short-form.
				[arrrgh::Optional|arrrgh::Required], // To indicate whether it's required.
				[default-value] 		// The default value. Defaults to 0 or its equivalent for this type.
			);

5.	Let 'er rip:

		parser.parse( argc, argv );

	Use try...catch... if you want to catch problems in a healthy way.

6.	Access argument values:

		myArgument.value()		// Returns a float if we templated on <float>.

	try...catch... helps here too because users may give invalid strings for non-string types.

7.	Call parser.show_usage() if you want to print help text.

8.	Do other stuff. See "Example usage" below for "unlabeled" arguments, the "--" marker, and such.

# Example Usage

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
			// (everything else becomes unlabeled).
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

# License

Released under the MIT license. See arrrgh.hpp.