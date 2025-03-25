#include "sdl_handler.h"
#include <algorithm>
void SDLHandler::handleEvent() {
  SDL_Event event;
  if (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: {
      SDL_Quit();
      shouldQuit = true;
      break;
    }
    }
  };
}

void SDLHandler::displayFrame(std::vector<std::vector<RGB>> display) {
  // printf("FRAME \n");
  // time(&end);
  end = std::chrono::steady_clock::now();

  // std::cout << "FPS: " <<
  // 1000.0/std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
  // << std::endl;
  unsigned char srcPixels[256 * 240 * 3];
  int index = 0;
  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 256; x++) {
      RGB pixel = display[x][y];
      // printf("INDEX: %d\n", index);
      srcPixels[index] = pixel.r;
      srcPixels[index + 1] = pixel.g;
      srcPixels[index + 2] = pixel.b;
      if (x % 8 == 0 || y % 8 == 0) {
        //    memset(&srcPixels[index], 255, 3);
      }
      index += 3;
      // if(y==239 && x==255)
      // spdlog::info("Index is: {0:d}", index);
      // SDL_SetRenderDrawColor(renderer, pixel.r, pixel.g, pixel.b, 1);
      // SDL_RenderDrawPoint(renderer, x, y);
    }
  }
  // Check this
  void *destPixels;
  int pitch;
  SDL_LockTexture(texture, NULL, &destPixels, &pitch);
  // spdlog::info("Pitch is : {0:d}", pitch);
  memcpy(destPixels, srcPixels, 256 * 240 * 3);
  SDL_UnlockTexture(texture);
  SDL_RenderCopy(renderer, texture, NULL,
                 NULL); // Update screen SDL_RenderPresent( gRenderer );
  SDL_RenderPresent(renderer);
  // start = std::chrono::steady_clock::now();
  // time(&start);
  // SDL_RenderPresent(renderer);
}

coro::task<void> SDLHandler::cpuLoop() {
  while (!shouldQuit) {
    nes->setTime(ticksInFrame);
    ticksInFrame = std::min(
        (unsigned int)18,
        SDL_GetTicks() - frameStartTicks); // TODO: fix this, frame calculation
                                           // seems problematic

    while (nes->hasCPUCycles()) {
      if (nes->ppuCanExecute()) {
        co_await nes->waitForPpuExecution();
      }

      nes->cpuCycle();
      while (nes->apuCyclesLeft()) {
        // spdlog::info("IN APU CYCLE");
        nes->apuCycle();
      }

      nes->resumePpuIfPossible();
      if (nes->shouldRender()) {
        handleEvent();
        displayFrame(nes->getFrame());
      }

      frameStartTicks = SDL_GetTicks();
    }
  }
}

coro::task<void> SDLHandler::ppuLoop() {
  while (!shouldQuit) {
    co_await nes->ppuCycle();
  }
}

void SDLHandler::begin() {
  coro::sync_wait(coro::when_all(cpuLoop(), ppuLoop()));
}
