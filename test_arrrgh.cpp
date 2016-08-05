//
//  main.cpp
//
//  Created by Jeff Wofford on 7/2/14.
//  Copyright (c) 2014 Jeff Wofford. All rights reserved.
//

#include "arrrgh.hpp"
#include <iostream>

namespace
{
	const bool VERBOSE = true;
	
	struct program
	{
		program( const std::string& program, const std::string& description )
		:	m_name( program )
		,	m_parser( program, description )
		{}
		
		const std::string& name() const { return m_name; }
		arrrgh::parser& parser() { return m_parser; }
		
		bool test( int argc, const char* argv[], bool expectFailure )
		{
			m_parser.clear_values();
						
			try
			{
				m_parser.parse( argc, argv );
			}
			catch( const std::exception& e )
			{
				if( VERBOSE )
				{
					std::cerr << "Error parsing arguments: " << e.what() << std::endl;
				}
				return expectFailure;
			}
			
			try
			{
				m_parser.each_argument( [&]( const arrrgh::argument_abstract& arg )
				{
					const std::string value = arg.converted_value_string();
					
					if( VERBOSE )
					{
						std::cout << "Argument " << arg.best_name() << "=" << value << std::endl;
					}
				} );
				
				m_parser.each_unlabeled_argument( [&]( const std::string& arg )
											   {
												   if( VERBOSE )
												   {
													   std::cout << "Unlabeled: \"" << arg << "\"" << std::endl;
												   }
											   } );
			}
			catch( const std::exception& e )
			{
				if( VERBOSE )
				{
					std::cerr << "Error reading argument values: " << e.what() << std::endl;
				}
				return expectFailure;
			}
			
			if( VERBOSE )
			{
				if( expectFailure )
				{
					std::cout << m_name << " failed to fail.\n";
				}
			}
			
			return !expectFailure;
		}
	
		bool test( std::string commandLine, bool expectFailure )
		{
			if( VERBOSE )
			{
				std::cout << "Testing " << m_name << ": '" << commandLine << "' expecting " << ( expectFailure ? "failure" : "success" ) << "..." << std::endl;
			}
			
			// Prepend program name.
			
			commandLine = m_name + " " + commandLine;
			
			// Break command line into arguments.
						
			const char* const rawCommandLine = commandLine.c_str();
			const char* const endCommandLine = rawCommandLine + commandLine.size();
			
			std::vector< std::string > args;

			bool quoted = false;
			bool skippingSpace = false;
			
			const char* tokenStart = rawCommandLine;
			for( const char* pc = tokenStart; pc != endCommandLine; ++pc )
			{
				const char c = *pc;
				
				if( !quoted && std::isspace( c ))
				{
					if( !skippingSpace )
					{
						// Here endeth a token. Copy it.
						//
						std::string arg;
						std::copy( tokenStart, pc, std::back_inserter( arg ));
						
						args.emplace_back( std::move( arg ));
					}
					
					skippingSpace = true;
				}
				else
				{
					if( skippingSpace )
					{
						// Start of new token.
						//
						tokenStart = pc;
					}
					skippingSpace = false;
					
					if( c == '\"' )
					{
						quoted = !quoted;
					}
				}
			}
			
			if( !skippingSpace )
			{
				// Copy the last token.
				//
				std::string arg;
				std::copy( tokenStart, endCommandLine, std::back_inserter( arg ));
				args.emplace_back( std::move( arg ));
			}
			
			// Convert arg strings to array of pointers.
			//
			std::vector< const char* > rawArgs;
			std::transform( args.begin(), args.end(), std::back_inserter( rawArgs ), []( const std::string& s ) { return s.c_str(); } );
			
			return test( static_cast< int >( args.size() ), rawArgs.data(), expectFailure );
		}
		
	private:
		
		std::string m_name;
		arrrgh::parser m_parser;
	};
}

#define VERIFY_TEST( expr )	if( !(expr) ) std::cout << "Test failed: " << #expr << std::endl;
#define VERIFY_EQUAL( exprA, exprB )	if( !( (exprA) == (exprB) )) std::cout << "Test failed: \"" << (exprA) << "\" != \"" << (exprB) << "\"" << std::endl;

void testProgramWithNoArgs()
{
	program program( "no_args", "Tests program with no arguments." );
	
	// EXPECT SUCCESS: No arguments passed in.
	//
	if( !program.test( "", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
	}
	
	// EXPECT FAILURE: Unwanted arguments passed in.
	//
	if( !program.test( "-a --switch=purple", true ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
	}
	
	// EXPECT SUCCESS: Test with unexpected unlabeled arguments passed in. This is okay.
	//
	if( !program.test( "file1.jpg", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
	}
}

void testSingleOptionalBoolArg()
{
	program program( "args1b", "Tests a single optional bool argument." );
	
	const auto& arg = program.parser().add< bool >( "long", "Really awesome description.", 'l' );
	
	// EXPECT SUCCESS: No arguments passed in.
	//
	if( !program.test( "", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_TEST( !arg.value() );
	}
	
	// EXPECT SUCCESS: Short form argument passed in.
	//
	if( !program.test( "-l", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_TEST( arg.value() );
	}
	
	// EXPECT SUCCESS: Long form argument passed in.
	//
	if( !program.test( "--long", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_TEST( arg.value() );
	}
	
	// EXPECT FAILURE: Missing hyphen before long form.
	//
	if( !program.test( "-long", true ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
	}
}

void testSetOfBoolArg()
{
	program program( "args3b", "Tests sets of bool args." );
	
	const auto& argA = program.parser().add< bool >( "", "a.", 'a' );
	const auto& argB = program.parser().add< bool >( "", "b.", 'b' );
	const auto& argC = program.parser().add< bool >( "", "c.", 'c' );
	
	// EXPECT SUCCESS: No arguments passed in.
	//
	if( !program.test( "", false ) )
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_TEST( !argA.value() );
		VERIFY_TEST( !argB.value() );
		VERIFY_TEST( !argC.value() );
	}
	
	// EXPECT SUCCESS: Independent arguments passed in.
	//
	if( !program.test( "-a -c", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_TEST( argA.value() );
		VERIFY_TEST( !argB.value() );
		VERIFY_TEST( argC.value() );
	}
	
	// EXPECT SUCCESS: Collection of arguments passed in.
	//
	if( !program.test( "-bc", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_TEST( !argA.value() );
		VERIFY_TEST( argB.value() );
		VERIFY_TEST( argC.value() );
	}
}

void testOptionalNumericArg()
{
	program program( "args1f", "Tests optional numeric argument." );
	
	const auto& arg = program.parser().add< double >( "weight", "The weight of the thing.", 'w' );
	
	// EXPECT SUCCESS: No arguments passed in.
	//
	if( !program.test( "", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_EQUAL( arg.value(), 0 );
	}
	
	// EXPECT FAILURE: Passed without assignment
	//
	if( !program.test( "-w", true ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
	}
	
	// EXPECT SUCCESS: Assigned with short form.
	//
	if( !program.test( "-w=4.5", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_EQUAL( arg.value(), 4.5 );
	}
	
	// EXPECT SUCCESS: Assigned with long form.
	//
	if( !program.test( "--weight=128", false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_EQUAL( arg.value(), 128 );
	}
}

void testUnlabeledArgs()
{
	program program( "args_unlabeled", "Tests unlabeled arguments." );
	
	// EXPECT SUCCESS: Several arguments passed.
	//
	const std::string givenArgString = "aardvark b Caveman";
	if( !program.test( givenArgString, false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		std::string argString;
		program.parser().each_unlabeled_argument( [&]( const std::string& arg )
												 {
													 argString += arg;
													 argString += " ";
												 } );
		
		argString.pop_back();	// Final space.
		
		VERIFY_EQUAL( givenArgString, argString );
	}
}

void testEverything( bool withEqualSign )
{
	program program( "enchilada", "The whole enchilada: a mix of all argument types and situations." );
	
	const auto& argA = program.parser().add< bool >( "", "a.", 'a' );
	const auto& argB = program.parser().add< bool >( "", "b.", 'b' );
	const auto& argC = program.parser().add< bool >( "", "c.", 'c' );
	const auto& argD = program.parser().add< bool >( "", "d.", 'd' );
	const auto& argF = program.parser().add< float >( "float", "f.", 'f', arrrgh::Optional, 3.141f );
	const auto& argG = program.parser().add< float >( "groat-moat", "g.", 'g' );
	const auto& argH = program.parser().add< int >( "height", "h.", 'h', arrrgh::Required );
	const auto& argS = program.parser().add< std::string >( "name", "s.", 's', arrrgh::Required );

	auto argBreak = [withEqualSign]()
	{
		return withEqualSign ? "=" : " ";
	};

	const std::string args = std::string( "-a --groat-moat" ) + argBreak() + "16.25 -bd unlabeled --name" + argBreak() + "\"Absolom, Absolom\" -h" + argBreak() + "8 -- --weirdly-unlabeled";
	
	if( !program.test( args, false ))
	{
		std::cout << program.name() << " FAILED" << std::endl;
	}
	else
	{
		if( VERBOSE ) std::cout << program.name() << " succeeded.\n";
		VERIFY_TEST( argA.value() );
		VERIFY_TEST( argB.value() );
		VERIFY_TEST( !argC.value() );
		VERIFY_TEST( argD.value() );
		VERIFY_EQUAL( argF.value(), 3.141f );
		VERIFY_EQUAL( argG.value(), 16.25f );
		VERIFY_EQUAL( argH.value(), 8 );
		
		// Note: The command line processor (e.g. bash) actually
		// removes the quotes, but we're too lazy to do that in our command line tokenizer.
		VERIFY_EQUAL( argS.value(), "\"Absolom, Absolom\"" );
		
		std::string argString;
		program.parser().each_unlabeled_argument( [&]( const std::string& arg )
												 {
													 argString += arg;
													 argString += " ";
												 } );
		
		argString.pop_back();	// Final space.

		VERIFY_EQUAL( argString, "unlabeled --weirdly-unlabeled" );
	}
}

int main( int argc, const char* argv[] )
{
	testProgramWithNoArgs();
	testSingleOptionalBoolArg();
	testSetOfBoolArg();
	testOptionalNumericArg();
	testUnlabeledArgs();
	testEverything( true /* with equal sign */ );

	std::cout << "Done.\n";
	
	return 0;
}

