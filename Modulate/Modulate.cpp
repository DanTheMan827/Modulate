﻿// Modulate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <sys/types.h>
#include <direct.h>
#include <windows.h>
#include <functional>

#include <deque>
#include <string>

#include "Error.h"
#include "Settings.h"
#include "Utils.h"

#include "CArk.h"
#include "CDtaFile.h"

eError EnableVerbose( std::deque< std::string >& )
{
    CSettings::mbVerbose = true;

    std::cout << "Verbose mode enabled\n";

    return eError_NoError;
}

eError EnableForceWrite( std::deque< std::string >& )
{
    CSettings::mbOverwriteOutputFiles = true;

    VERBOSE_OUT( "Overwriting existing files when necessary\n" );

    return eError_NoError;
}

eError BuildSingleSong( std::deque< std::string >& laParams )
{
    std::cout << "Building song ";
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }

    std::string lDirectory = laParams.front();
    laParams.pop_front();

    if( lDirectory.back() != '/' && lDirectory.back() != '\\' )
    {
        lDirectory += '/';
    }
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }

    std::string lSongName = laParams.front();
    laParams.pop_front();
    std::cout << lSongName.c_str() << "\n";

    std::string lFilename = lDirectory + "ps3/songs/" + lSongName + "/" + lSongName + ".moggsong";

    CDtaFile lDataFile;
    eError leError = lDataFile.LoadMoggSong( lFilename.c_str() );
    SHOW_ERROR_AND_RETURN;

    lDataFile.SetMoggPath( lSongName + ".mogg" );
    lDataFile.SetMidiPath( lSongName + ".mid" );

    std::string lOutFilename = lFilename + "_dta_ps3";

    leError = lDataFile.Save( lOutFilename.c_str() );
    SHOW_ERROR_AND_RETURN;

    return eError_NoError;
}

eError BuildSongs( std::deque< std::string >& laParams )
{
    std::cout << "Building songs at ";
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }

    std::string lDirectory = laParams.front();
    laParams.pop_front();

    if( lDirectory.back() != '/' && lDirectory.back() != '\\' )
    {
        lDirectory += '/';
    }
    std::cout << lDirectory.c_str() << "ps3/songs/\n";

    CArk lReferenceArkHeader;
    eError leError = lReferenceArkHeader.Load( "main_ps3.hdr" );
    SHOW_ERROR_AND_RETURN;

    std::vector< std::string > laFilenames;
    laFilenames.reserve( lReferenceArkHeader.GetNumFiles() );

    const char* lpMoggSong = ".moggsong";
    const size_t kiMoggSongStrLen = strlen( lpMoggSong );

    CUtils::GenerateFileList( lReferenceArkHeader, lDirectory.c_str(), laFilenames, lpMoggSong );
    for( std::vector< std::string >::const_iterator lIter = laFilenames.begin(); lIter != laFilenames.end(); ++lIter )
    {
        const std::string& lFilename = *lIter;
        const char* lpExtension = strstr( lFilename.c_str(), lpMoggSong );
        if( lpExtension != ( lFilename.c_str() + strlen( lFilename.c_str() ) - kiMoggSongStrLen ) )
        {
            continue;
        }

        const char* kpSearchString = "/songs/";
        const char* lpSongName = strstr( lFilename.c_str(), kpSearchString );
        if( !lpSongName )
        {
            continue;
        }
        lpSongName += strlen( kpSearchString );

        const char* lpSongNameEnd = strstr( lpSongName, "/" );
        if( !lpSongNameEnd )
        {
            return eError_InvalidParameter;
        }

        if( strstr( lpSongName, "tut" ) == lpSongName && ( lpSongNameEnd - lpSongName ) == 4 )
        {
            continue;
        }

        std::string lSongName( lpSongName );
        lSongName = lSongName.substr( 0, lpSongNameEnd - lpSongName );

        std::deque< std::string > laSingleSongParam = { lDirectory, lSongName };
        eError leError = BuildSingleSong( laSingleSongParam );
        SHOW_ERROR_AND_RETURN;
    }

    return eError_NoError;
}

eError Unpack( std::deque< std::string >& laParams )
{
    std::cout << "Unpacking main_ps3.hdr to ";
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }

    std::string lOutputDirectory = laParams.front();
    laParams.pop_front();

    std::cout << lOutputDirectory.c_str() << "\n";
    if( lOutputDirectory.back() != '/' && lOutputDirectory.back() != '\\' )
    {
        lOutputDirectory += "/";
    }

    CArk lArkHeader;
    eError leError = lArkHeader.Load( "main_ps3.hdr" );
    SHOW_ERROR_AND_RETURN;

    leError = lArkHeader.ExtractFiles( 0, lArkHeader.GetNumFiles(), lOutputDirectory.c_str() );
    SHOW_ERROR_AND_RETURN;

    return eError_NoError;
}

eError ReplaceSong( std::deque< std::string >& laParams )
{
    std::cout << "Replacing song ";
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }
    std::string lBasePath = laParams.front();
    laParams.pop_front();
    if( lBasePath.back() != '/' && lBasePath.back() != '\\' )
    {
        lBasePath += "/";
    }

    std::cout << lBasePath.c_str();
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }
    std::string lOldSong = laParams.front();
    laParams.pop_front();

    std::cout << lOldSong.c_str() << " with " << lBasePath.c_str();
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }
    std::string lNewSong = laParams.front();
    laParams.pop_front();

    std::cout << lNewSong.c_str() << "\n";

    std::string lSourcePathBase = lBasePath + "ps3/songs/" + lNewSong + "/" + lNewSong;
    std::string lTargetPathBase = lBasePath + "ps3/songs/" + lOldSong + "/" + lOldSong;

    auto lCopyFile = [ & ]( const char* lpExtension )
    {
        std::string lSourcePath = lSourcePathBase + lpExtension;
        std::string lTargetPath = lTargetPathBase + lpExtension;

        std::cout << lSourcePath << " --> " << lTargetPath.c_str() << "\n";
        if( !CopyFileA( lSourcePath.c_str(), lTargetPath.c_str(), FALSE ) )
        {
            return eError_FailedToCreateFile;
        }

        return eError_NoError;
    };

    eError leError = lCopyFile( ".mid     " );   SHOW_ERROR_AND_RETURN;
    leError = lCopyFile( ".mogg    " );          SHOW_ERROR_AND_RETURN;
    leError = lCopyFile( ".moggsong" );          SHOW_ERROR_AND_RETURN;

    std::string lTarget = lTargetPathBase + ".moggsong";

    std::deque< std::string > lSingleSongParams = { lBasePath, lOldSong };
    BuildSingleSong( lSingleSongParams );

    return eError_NoError;
}

eError Pack( std::deque< std::string >& laParams )
{
    std::cout << "Packing main_ps3.hdr from ";
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }

    std::string lInputPath = laParams.front();
    laParams.pop_front();
    std::cout << lInputPath.c_str() << " to ";
    if( laParams.empty() )
    {
        return eError_InvalidParameter;
    }

    std::string lOuptutPath = laParams.front();
    laParams.pop_front();
    std::cout << lOuptutPath.c_str() << "\n";

    if( lInputPath.back() != '/' && lInputPath.back() != '\\' )
    {
        lInputPath += "/";
    }
    if( lOuptutPath.back() != '/' && lOuptutPath.back() != '\\' )
    {
        lOuptutPath += "/";
    }

    CArk lReferenceArkHeader;
    eError leError = lReferenceArkHeader.Load( "main_ps3.hdr" );
    SHOW_ERROR_AND_RETURN;

    CArk lArkHeader;
    leError = lArkHeader.ConstructFromDirectory( lInputPath.c_str(), lReferenceArkHeader );
    SHOW_ERROR_AND_RETURN;

    lArkHeader.BuildArk( lInputPath.c_str() );
    lArkHeader.SaveArk( lOuptutPath.c_str(), "main_ps3.hdr" );

    return eError_NoError;
}

void PrintUsage()
{
    std::cout << "Usage: Modulate.exe <options> <command>\n\n";
    std::cout << "Options:\n";
    std::cout << "-verbose\t\t\tShow additional information during operation\n";
    std::cout << "-force\t\t\t\tOverwrite existing files\n";
    std::cout << "\nCommands:\n";
    std::cout << "-unpack <out_dir>\t\tUnpack the contents of the .ark file(s) to <out_dir>\n";
    std::cout << "-replace <old_song> <new_song>\tReplace the song in the <old_song> folder with the song in <new_song>.\n\t\t\t\tRequires <new_song.mid> <new_song.mogg> <new_song.moggsong>\n";
    std::cout << "-pack <in_dir> <out_dir>\tRepack data into an .ark file\n";
}

int main( int argc, char *argv[], char *envp[] )
{
    struct sCommandPair
    {
        const char* mpCommandName;
        std::function<eError( std::deque< std::string >& )> mFunction;
    };
    const sCommandPair kaCommands[] = {
        "-verbose",     EnableVerbose,
        "-force",       EnableForceWrite,
        "-unpack",      Unpack,
        "-replace",     ReplaceSong,
        "-buildsong",   BuildSingleSong,
        "-buildsongs",  BuildSongs,
        "-pack",        Pack,
        nullptr,        nullptr,
    };

    std::deque< std::string > laParams;
    for( int ii = 1; ii < argc; ++ii )
    {
        laParams.push_back( argv[ ii ] );
    }

    if( laParams.empty() )
    {
        PrintUsage();
        return 0;
    }

    while( !laParams.empty() )
    {
        size_t liNumParams = laParams.size();

        const sCommandPair* lpCommandPair = kaCommands;
        while( lpCommandPair->mFunction )
        {
            if( _stricmp( laParams.front().c_str(), lpCommandPair->mpCommandName ) != 0 )
            {
                ++lpCommandPair;
                continue;
            }

            laParams.pop_front();
            eError leError = lpCommandPair->mFunction( laParams );
            switch( leError )
            {
            case eError_NoError:
                std::cout << "\n";
                break;
            default:
                ShowError( leError );
                return -1;
            }

            break;
        }

        if( liNumParams == laParams.size() )
        {
            std::cout << "Unkown parameter: " << laParams.front().c_str() << "\nAborting\n\n";
            PrintUsage();
            return -1;
        }
    }

    std::cout << "Complete!\n";

    return 0;
}
