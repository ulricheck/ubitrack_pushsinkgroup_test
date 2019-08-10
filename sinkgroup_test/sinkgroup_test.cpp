/*
 * Ubitrack - Library for Ubiquitous Tracking
 * Copyright 2006, Technische Universitaet Muenchen, and individual
 * contributors as indicated by the @authors tag. See the
 * copyright.txt in the distribution for a full listing of individual
 * contributors.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */


#include <stdlib.h>
#include <signal.h>
#include <iostream>
#ifdef _WIN32
#include <conio.h>
#endif
#include <boost/thread.hpp>
#include <boost/program_options.hpp>

#include <utFacade/AdvancedFacade.h>
#include <utFacade/SinkGroup/SinkGroup.h>
#include <utFacade/SinkGroup/SinkGroupObserver.h>
#include <utUtil/Exception.h>
#include <utUtil/Logging.h>
#include <utUtil/OS.h>

using namespace Ubitrack;


bool bStop = false;


void ctrlC ( int i )
{
	bStop = true;
}


int main( int ac, char** av )
{
	signal ( SIGINT, &ctrlC );
	
	try
	{
		// initialize logging		
		Util::initLogging();
		

		// program options
		std::string sServerAddress;
		std::string sUtqlFile;
		std::string sExtraUtqlFile;
		std::string sComponentsPath;
		bool bNoExit;

		bool dropEvents=true;

		try
		{
			// describe program options
			namespace po = boost::program_options;
			po::options_description poDesc( "Allowed options", 80 );
			poDesc.add_options()
				( "help", "print this help message" )
				( "server,s", po::value< std::string >( &sServerAddress ), "Ubitrack server address <host>[:<port>] to connect to" )
				( "components_path", po::value< std::string >( &sComponentsPath ), "Directory from which to load components" )
				( "utql", po::value< std::string >( &sUtqlFile ), "UTQL request or response file, depending on whether a server is specified. "
					"Without specifying this option, the UTQL file can also be given directly on the command line." )
				( "extra-dataflow", po::value< std::string >( &sExtraUtqlFile ), "Additional UTQL response file to be loaded directly without using the server" )
				( "noexit", "do not exit on return" )
				( "path", "path to ubitrack bin directory" )
				("path", "path to ubitrack bin directory")
				("allEvents", "should all events be computed, no drops")
				#ifdef _WIN32
				( "priority", po::value< int >( 0 ),"set priority of console thread, -1: lower, 0: normal, 1: higher, 2: real time (needs admin)" )
				#endif
			;
			
			// specify default options
			po::positional_options_description inputOptions;
			inputOptions.add( "utql", 1 );		
			
			// parse options from command line and environment
			po::variables_map poOptions;
			po::store( po::command_line_parser( ac, av ).options( poDesc ).positional( inputOptions ).run(), poOptions );
			po::store( po::parse_environment( poDesc, "UBITRACK_" ), poOptions );
			po::notify( poOptions );

			#ifdef _WIN32
			// set to quite high priority, see: http://msdn.microsoft.com/en-us/library/windows/desktop/ms686219%28v=vs.85%29.aspx
			// carefully use this function under windows to steer ubitrack's cpu time:
			if(poOptions.count("priority") != 0) {
				
				int prio = poOptions["priority"].as<int>();
				switch(prio){
				case -1:
					SetPriorityClass( GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS );
					break;
				case 0:
					//SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS  );
					break;
				case 1:
					SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS  );
					break;
				case 2:
					SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS  );
					break;
				}
			}
			#endif

			bNoExit = poOptions.count( "noexit" ) != 0;

			dropEvents = poOptions.count("allEvents") == 0;
			
			// print help message if nothing specified
			if ( poOptions.count( "help" ) || sUtqlFile.empty() )
			{
				std::cout << "Syntax: utConsole [options] [--utql] <UTQL file>" << std::endl << std::endl;
				std::cout << poDesc << std::endl;
				return 1;
			}
			
			
		}
		catch( std::exception& e )
		{
			std::cerr << "Error parsing command line parameters : " << e.what() << std::endl;
			std::cerr << "Try utConsole --help for help" << std::endl;
			return 1;
		}		
		
		// configure ubitrack
		

		std::cout << "Droping events:" << dropEvents << std::endl << std::flush;
		std::cout << "Loading components..." << std::endl << std::flush;
		Facade::AdvancedFacade utFacade(dropEvents, sComponentsPath  );

		if ( sServerAddress.empty() )
		{
			std::cout << "Instantiating dataflow network from " << sUtqlFile << "..." << std::endl << std::flush;
			utFacade.loadDataflow( sUtqlFile );
		}
		else
		{
			if ( !sExtraUtqlFile.empty() )
				utFacade.loadDataflow( sExtraUtqlFile, false );
				
			std::cout << "Connecting to server " << sServerAddress << "..." << std::endl << std::flush;
			utFacade.connectToServer( sServerAddress );
			
			std::cout << "Sending UTQL to to server " << sUtqlFile << "..." << std::endl << std::flush;
			utFacade.sendUtqlToServer( sUtqlFile );
		}

		std::cout << "Starting dataflow" << std::endl;
		utFacade.startDataflow();

		while( !bStop )
		{
			Util::sleep( 1000 );
			#ifdef _WIN32
			if(kbhit())
			{
				char c = getch();		
				if(c == 'q') bStop = true;
			}
			#endif
		}

		std::cout << "Stopping dataflow..." << std::endl << std::flush;
		utFacade.stopDataflow();

		std::cout << "Finished, cleaning up..." << std::endl << std::flush;
	}
	catch( Util::Exception& e )
	{
		std::cout << "exception occurred" << std::endl << std::flush;
		std::cerr << e << std::endl;
	}

	std::cout << "utConsole terminated." << std::endl << std::flush;
}
