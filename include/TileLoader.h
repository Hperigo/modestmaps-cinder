#pragma once

#include <set>
#include <map>
#include "cinder/Cinder.h"
#include "cinder/Thread.h"
#include "cinder/gl/Texture.h"
#include "Coordinate.h"
#include "MapProvider.h"

namespace cinder { namespace modestmaps {

// limit simultaneous calls to loadImage
#define MAX_PENDING 2

class TileLoader;
typedef std::shared_ptr<TileLoader> TileLoaderRef;

class TileLoader
{
	
private:
    TileLoader();
    TileLoader( MapProviderRef _provider ); /*: provider(_provider) {}*/
    
    void doThreadedPaint( );
    
	std::mutex pendingCompleteMutex;	
	std::set<Coordinate> pending;
	std::map<Coordinate, Surface> completed;
    
    MapProviderRef provider;
    
    std::shared_ptr<std::thread> mThreadRef;
    std::atomic<bool> runThread;
    
public:
    ~TileLoader();
    static TileLoaderRef create( MapProviderRef provider )
    {
        
//        return std::make_shared<TileLoader>(new TileLoader(provider) );
        return TileLoaderRef( new TileLoader( provider ) );
    }
    
	void processQueue( std::vector<Coordinate> &queue );
	
	void transferTextures( std::map<Coordinate, gl::Texture> &images);

	bool isPending(const Coordinate &coord);
    
    void setMapProvider( MapProviderRef _provider );
    
};
    
} } // namespace