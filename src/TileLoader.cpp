/*
 *  TileLoader.cpp
 *  modestmaps-ci
 *
 *  Created by Tom Carden on 8/27/10.
 *  Copyright 2010 Stamen Design. All rights reserved.
 *
 */

#include "TileLoader.h"
#include "cinder/app/App.h"

#if defined( CINDER_COCOA )
#include <objc/objc-auto.h>
#endif

namespace cinder { namespace modestmaps {

TileLoader::TileLoader(){
    
}

TileLoader::~TileLoader(){
    
    runThread = false;
    mThreadRef->join();
}
    
TileLoader::TileLoader(MapProviderRef _provider) : provider(_provider){
    
    runThread = true;
    mThreadRef = std::make_shared<std::thread>([&]{ doThreadedPaint(); } );
    
    
}
    
    
void TileLoader::doThreadedPaint()
{
    
    ci::app::console() << std::thread::hardware_concurrency() << std::endl;
    
    while(runThread){
        Surface image;
        
        
        pendingCompleteMutex.lock();
        Coordinate coord = *pending.begin();
        pendingCompleteMutex.unlock();
        
        if (provider) {
            image = provider->createSurface( coord );
        }
        
        pendingCompleteMutex.lock();
        if (pending.count(coord) > 0) {
            if (image) {
                completed[coord] = image;
            }
            pending.erase(coord);  
        } // otherwise clear was called so we should abandon this image to the ether
        
        pendingCompleteMutex.unlock();
    }
}

void TileLoader::processQueue(std::vector<Coordinate> &queue )
{
	while (pending.size() < MAX_PENDING && queue.size() > 0) {
		Coordinate key = *(queue.begin());
		queue.erase(queue.begin());

        pendingCompleteMutex.lock();
        pending.insert(key);
        pendingCompleteMutex.unlock();
	}
}

void TileLoader::transferTextures(std::map<Coordinate, gl::Texture> &images)
{
    // use try_lock because we can just wait until next frame if needed
    if (pendingCompleteMutex.try_lock()) {
        if (!completed.empty()) {
            std::map<Coordinate, Surface>::iterator iter = completed.begin();
            if (iter->second) {
                images[iter->first] = gl::Texture(iter->second);		
            }
            completed.erase(iter);
        }
        pendingCompleteMutex.unlock();
    }
}
    
bool TileLoader::isPending(const Coordinate &coord)
{
    bool coordIsPending = false;
    pendingCompleteMutex.lock();
    coordIsPending = (pending.count(coord) > 0);
    pendingCompleteMutex.unlock();
    return coordIsPending;
}
    
void TileLoader::setMapProvider( MapProviderRef _provider )
{
	pendingCompleteMutex.lock();
    completed.clear();
    pending.clear();
	pendingCompleteMutex.unlock();
    provider = _provider;
}

} } // namespace