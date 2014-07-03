//
//  arrrgh.hpp
//
//  Created by Jeff Wofford on 7/2/14.
//  Copyright (c) 2014 Jeff Wofford. All rights reserved.
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//	
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//	
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//

#ifndef jeffwofford_arrrgh_hpp_included
#define jeffwofford_arrrgh_hpp_included

#include <cassert>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace arrrgh
{
	// Utility functions and macros.
	//
	template< typename Function >
	std::string collect_string( Function&& fn )
	{
		return fn();
	}
#define arrrgh_collect_string( expression ) arrrgh::collect_string( [&]() { std::ostringstream stream; stream << expression; return stream.str(); } )
	
#define ARRRGH_EXCEPTION( exception_class ) struct exception_class : public std::runtime_error { using runtime_error::runtime_error;  };
	
	enum Requirement
	{
		Optional,
		Required
	};
	
	template< typename ValueT >
	struct type_traits
	{
		static constexpr bool always_requires_value() { return false; }
		static constexpr const char* name();
	};
	
	// argument classes.
	//
	class argument_abstract
	{
	public:
		
		ARRRGH_EXCEPTION( MissingValue )
		
		std::string best_name() const
		{
			if( m_longForm.empty() )
			{
				return std::string{ m_letter };
			}
			else
			{
				return m_longForm;
			}
		}
		
		bool assigned() const
		{
			return m_assigned;
		}
		
		const std::string& value_string() const
		{
			return m_value;
		}
		
		void clear_value()
		{
			m_assigned = false;
			m_value.clear();
		}

		// FOR TESTING.
		// Gets the string value of the argument as a result of its conversion to the
		// templated value_t for that particular argument subclass.
		//
		virtual std::string converted_value_string() const = 0;
		virtual bool has_default_value() const = 0;
		
	protected:
		
		bool m_assigned;
		
		explicit argument_abstract( const std::string& longForm,
								    const std::string& explanation,
								    char letter,
								    Requirement required )
		:	m_longForm( longForm )
		,	m_explanation( explanation )
		,	m_letter( letter )
		,	m_requirement( required )
		,	m_assigned( false )
		{}
		
		bool has_long_form( const std::string& longForm ) const
		{
			return !m_longForm.empty() && m_longForm == longForm;
		}
		
		bool has_short_form( char shortForm ) const
		{
			return m_letter != '\0' && m_letter == shortForm;
		}
		
		bool required() const
		{
			return m_requirement == Required;
		}
		
		virtual bool required_value() const = 0;

		void assign( const std::string& givenKey, std::string&& valueString )
		{
			m_assigned = true;
			
			m_value = std::move( valueString );
			
			if( m_value.empty() && required_value() )
			{
				throw MissingValue{ arrrgh_collect_string( "Argument required a value but received none." ) };
			}
		}
								
		void print( std::ostream& out ) const
		{
			out << "    ";
			
			bool hasLetter = m_letter != '\0';
			if( hasLetter )
			{
				out << "-" << m_letter;
			}
			
			bool hasLong = !m_longForm.empty();
			
			if( hasLetter && hasLong )
			{
				out << ", ";
			}
			
			if( hasLong )
			{
				out << "--" << m_longForm;
			}
			
			if( required_value() )
			{
				out << "=<" << value_type_name() << ">";
			}
			
			out << "\n\t\t";
			
			if( m_requirement == Required )
			{
				out << "[required] ";
			}
			
			out << m_explanation;
		}
		
		virtual std::string value_type_name() const = 0;
		
	private:
		
		std::string m_longForm;
		std::string m_explanation;
		char m_letter;
		Requirement m_requirement;
		
		std::string m_value;
		
		friend class parser;
	};
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	template< typename ValueType >
	class argument : public argument_abstract
	{
	public:
		typedef ValueType value_t;
		
		// Exception types.
		//
		ARRRGH_EXCEPTION( ValueConversionError )

		value_t value() const
		{
			value_t result = m_defaultValue;
			
			if( m_assigned )
			{
				std::istringstream stream( value_string() );
				stream >> std::boolalpha >> result;
				
				if( stream.fail() )
				{
					throw ValueConversionError{
						arrrgh_collect_string( "Could not convert value '" << value_string()
						<< "' to the desired argument type." )};
				}
			}
			
			return result;
		}
		
	private:
		
		value_t m_defaultValue = value_t{};

		explicit argument( const std::string& longForm,
						   const std::string& explanation,
						   char letter,
						   Requirement required,
						   const value_t& defaultValue )
		:	argument_abstract( longForm, explanation, letter, required )
		,	m_defaultValue( defaultValue )
		{}
		
		virtual std::string value_type_name() const override
		{
			return type_traits< value_t >::name();
		}
		
		virtual std::string converted_value_string() const override
		{
			std::ostringstream stream;
			stream << std::boolalpha << value();
			return stream.str();
		}
		
		virtual bool has_default_value() const override
		{
			return value() == m_defaultValue;
		}
		
		virtual bool required_value() const override
		{
			return type_traits< value_t >::always_requires_value();
		}

		friend class parser;
	};
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	class parser
	{
	public:
		
		// Exception classes.
		//
		ARRRGH_EXCEPTION( InvalidParameters )
		ARRRGH_EXCEPTION( UnknownArgument )
		ARRRGH_EXCEPTION( FoundDoubleHyphensLeadingNowhere )
		ARRRGH_EXCEPTION( InvalidArgumentCharacter )
		ARRRGH_EXCEPTION( MissingRequiredArguments )
		
		explicit parser( const std::string& programName, const std::string& programDescription )
		:	m_program( programName )
		,	m_description( programDescription )
		{}

		template< typename ValueT >
		argument< ValueT >& add( const std::string& longForm,
								 const std::string& explanation,
								 char letter = '\0',
								 Requirement required = Optional,
								 const ValueT& defaultValue = ValueT{} )
		{
			// Verify that no other argument has this longForm or letter.
			//
			assert( !std::any_of( m_arguments.begin(), m_arguments.end(), [&]( const std::unique_ptr< argument_abstract >& arg )
								 {
									 assert( arg );
									 return arg->has_long_form( longForm ) || arg->has_short_form( letter );
								 } ));
			
			argument< ValueT >* arg = new argument< ValueT >{
				longForm,
				explanation,
				letter,
				required,
				defaultValue };
			
			m_arguments.emplace_back( arg );
			
			return *arg;
		}
		
		void parse( const int argc, const char* argv[] )
		{
			if( argc == 0 || !argv )
			{
				throw InvalidParameters{ "Received no arguments." };
			}

			m_programExecutionPath = argv[ 0 ];
			
			bool doneWithSwitches = false;		// When false, still looking for switches. When true, all arguments
												// are considered "unlabeled."
			
			for( int i = 1; i < argc; ++i )
			{
				assert( argv[ i ] );
				
				std::string arg{ argv[ i ] };
				assert( !arg.empty() );
				
				// Determine which configured argument corresponds to this program argument, if any,
				// and parse any value it might have.
				
				// Do we have a leading hyphen?
				//
				if( !doneWithSwitches && arg[ 0 ] == '-' )
				{
					// Yes we do.
					
					// Do we have two?
					//
					if( arg.size() > 1 && arg[ 1 ] == '-' )
					{
						// Yes. So we're expecting a long-form argument.
						
						// Read to the end or to the = sign.
						//
						const auto keyEnd = arg.find_first_of( '=' );
						
						const auto key = arg.substr( 2, keyEnd - 2 );
						
						std::string value;
						
						if( keyEnd < arg.size() )
						{
							value = arg.substr( keyEnd + 1 );
						}
						
						if( key.empty() )
						{
							// This is either "--" or, more troublingly, "--=..."
							
							// Either way, don't look for arguments any more:
							// anything else is "unlabeled."
							//
							doneWithSwitches = true;
							continue;
						}
						
						// Find this argument.
						//
						auto& argument = find_matching_argument( key, true /* long form */ );
						process_argument( argument, arg, std::move( value ));
					}
					else
					{
						// No we don't. Just one.
						
						// Consider each following letter to be a short-form argument letter.
						//
						for( size_t i = 1; i < arg.size(); ++i )
						{
							const char c = arg[ i ];
							
							// Is this a reasonable argument character?
							//
							if( isalpha( c ))
							{
								// This is a legitimate argument.
								
								// Does it have a value?
								//
								std::string value;
								bool hasAssignment = i + 1 < arg.size() && arg[ i + 1 ] == '=';
								if( hasAssignment )
								{
									// Looks like it. Read it.
									//
									value = arg.substr( i + 2 );
								}
								
								auto& argument = find_matching_argument( std::string{ c }, false /* short form */ );
								process_argument( argument, arg, std::move( value ));
								
								if( hasAssignment )
								{
									break;
								}
							}
							else
							{
								throw InvalidArgumentCharacter{
									arrrgh_collect_string( "Invalid argument with character '" << c << "'." ) };
							}
						}
					}
				}
				else
				{
					// No we don't. This is an unlabeled argument.
					//
					m_unlabeledArguments.emplace_back( std::move( arg ));
				}
			}
			
			// Did each of the *required* arguments get assigned?
			//
			if( std::any_of( m_arguments.begin(), m_arguments.end(),
							[&]( const std::unique_ptr< argument_abstract >& arg )
							{
								assert( arg );
								return !arg->assigned() && arg->required();
							} ))
			{
				throw MissingRequiredArguments{ "Some required arguments were missing." };
			}
		}
		
		void show_usage( std::ostream& out = std::cout ) const
		{
			out << m_program << ": " << m_description << std::endl;
			out << "usage: " << m_program << std::endl;
			
			for( const auto& arg : m_arguments )
			{
				assert( arg );
				arg->print( out );
				out << std::endl;
			}
			out << std::endl;
		}
		
		template< typename Function >
		void each_argument( Function&& fn ) const
		{
			std::for_each( m_arguments.begin(), m_arguments.end(), [&]( const std::unique_ptr< argument_abstract >& arg )
						  {
							  assert( arg );
							  fn( *arg );
						  } );
		}
		

		template< typename Function >
		void each_unlabeled_argument( Function&& fn ) const
		{
			std::for_each( m_unlabeledArguments.begin(), m_unlabeledArguments.end(), fn );
		}
		
		void clear_values()
		{
			std::for_each( m_arguments.begin(), m_arguments.end(), []( const std::unique_ptr< argument_abstract >& arg )
						  {
							  assert( arg );
							  arg->clear_value();
						  } );
			
			m_unlabeledArguments.clear();
		}
		
	protected:
		
		argument_abstract& find_matching_argument( const std::string& argString, bool longForm )
		{
			auto iterFound = std::find_if( m_arguments.begin(), m_arguments.end(),
										  [&]( const std::unique_ptr< argument_abstract >& arg )
										  {
											  assert( arg );
											  if( longForm )
											  {
												  return arg->has_long_form( argString );
											  }
											  else
											  {
												  return arg->has_short_form( argString[ 0 ] );
											  }
											
										  } );
			
			if( iterFound != m_arguments.end() )
			{
				return **iterFound;
			}
			else
			{
				throw UnknownArgument{ arrrgh_collect_string( "Unrecognized argument \"-" << ( longForm ? "-" : "" )
															 << argString << "\"." ) };
			}
		}
		
		void process_argument( argument_abstract& argument, const std::string& key, std::string&& value )
		{
			argument.assign( key, std::move( value ));
		}
		
	private:

		std::string m_description;
		std::string m_program;
		std::string m_programExecutionPath;
		std::vector< std::unique_ptr< argument_abstract >> m_arguments;
		std::vector< std::string > m_unlabeledArguments;
	};
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Specializations
	//
	template<>
	inline bool argument< bool >::value() const
	{
		return m_assigned;
	}

	template<>
	std::string argument< std::string >::value() const
	{
		return value_string();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// arrrgh::type_traits
	//
	template<>
	struct type_traits< bool >
	{
		static constexpr bool always_requires_value() { return false; }
		static constexpr const char* name() { return "bool"; }
	};

	template<>
	struct type_traits< std::string >
	{
		static constexpr bool always_requires_value() { return true; }
		static constexpr const char* name() { return "string"; }
	};
	
	template<>
	struct type_traits< float >
	{
		static constexpr bool always_requires_value() { return true; }
		static constexpr const char* name() { return "number"; }
	};
	
	template<>
	struct type_traits< double >
	{
		static constexpr bool always_requires_value() { return true; }
		static constexpr const char* name() { return "number"; }
	};
	
	template<>
	struct type_traits< int >
	{
		static constexpr bool always_requires_value() { return true; }
		static constexpr const char* name() { return "int"; }
	};
}

#undef arrrgh_collect_string
#undef ARRRGH_EXCEPTION

#endif
