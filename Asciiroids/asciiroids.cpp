//
//  asciiroids.cpp
//  Asciiroids
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#include "Keyboard.h"
#include "TitleScreen.h"
#include "InstructionsScreen.h"

#include <Termin8or/GameEngine.h>
#include <Termin8or/SpriteHandler.h>
#include <Termin8or/ASCII_Fonts.h>
#include <Termin8or/Dynamics/RigidBody.h>
#include <Termin8or/Dynamics/DynamicsSystem.h>
#include <Termin8or/Dynamics/CollisionHandler.h>
#include <8Beat/AudioSourceHandler.h>
#include <8Beat/ChipTuneEngine.h>

///////////////////////////////

class Game : public GameEngine<40, 100>
{
public:
  Game(int argc, char** argv, const GameEngineParams& params)
    : GameEngine(argv[0], params)
  {
  //#ifndef _WIN32
    GameEngine::set_real_fps(15);
    GameEngine::set_sim_delay_us(50'000);
    GameEngine::set_anim_rate(0, 5); // Spaceship
    GameEngine::set_anim_rate(1, 3); // Asteroids
    GameEngine::set_anim_rate(2, 5); // UFO AI
  //#endif
    if (argc >= 2)
      GameEngine::set_real_fps(static_cast<float>(atoi(argv[1])));
  }
  
  ~Game()
  {
    audio.remove_source(src_fx_shot);
    audio.remove_source(src_fx_explosion);
    audio.remove_source(src_fx_ufo_shot);
    audio.remove_source(src_fx_ufo_propulsion);
  }

  virtual void generate_data() override
  {
    try
    {
      std::string tune_path = get_exe_folder();
#ifndef _WIN32
      const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
      if (xcode_env != nullptr)
        tune_path = "../../../../../../../../Documents/xcode/Pilot_Episode/Pilot_Episode/"; // #FIXME: Find a better solution!
#endif
    
      if (chip_tune.load_tune(folder::join_path({ tune_path, "music.ct" })))
      {
          //chip_tune.play_tune();
          chip_tune.play_tune_async();
          chip_tune.wait_for_completion();
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    
    src_fx_shot = audio.create_stream_source();
    src_fx_explosion = audio.create_stream_source();
    src_fx_ufo_shot = audio.create_stream_source();
    src_fx_ufo_propulsion = audio.create_stream_source();
    
    std::string font_data_path = ASCII_Fonts::get_path_to_font_data(get_exe_folder());
    std::cout << font_data_path << std::endl;
    
    auto& cs0 = color_schemes.emplace_back();
    cs0.internal.fg_color = Color::Black;
    cs0.internal.bg_color = Color::Yellow;
    auto& cs1 = color_schemes.emplace_back();
    cs1.internal.fg_color = Color::White;
    cs1.internal.bg_color = Color::Black;
    
    font_data = ASCII_Fonts::load_font_data(font_data_path);
  }
  
private:

  virtual void update() override
  {
    Key curr_special_key [[maybe_unused]] = register_keypresses(kpdp);
    
    //update_ship_controls(sh, src_fx_0, wave_gen, kpdp, curr_special_key,
    //                         get_sim_dt_s());
    
    //draw_hud(sh, ...);
    
    draw_frame(sh, Color::LightGray);
    
    if (num_lives < 0)
      num_lives = 0;
    
    if (num_lives == 0)
      GameEngine::set_state_game_over();
    
    // Game logic.
    
    // Draw stuff.
  }
  
  virtual void on_quit() override
  {
    chip_tune.stop_tune_async();
  }
  
  virtual void draw_title() override
  {
    ::draw_title(sh, font_data, color_schemes[0]);
  }
  
  virtual void draw_instructions() override
  {
    ::draw_instructions(sh, font_data, color_schemes[1]);
  }
  
  virtual void on_exit_instructions() override
  {
    chip_tune.stop_tune_async();
  }

  //////////////////////////////////////////////////////////////////////////
    
  int num_lives = 3;
  
  audio::AudioSourceHandler audio;
  audio::WaveformGeneration wave_gen;
  audio::ChipTuneEngine chip_tune { audio, wave_gen };
  audio::AudioStreamSource* src_fx_shot = nullptr;
  audio::AudioStreamSource* src_fx_explosion = nullptr;
  audio::AudioStreamSource* src_fx_ufo_shot = nullptr;
  audio::AudioStreamSource* src_fx_ufo_propulsion = nullptr;
  
  std::vector<ASCII_Fonts::ColorScheme> color_schemes;
  ASCII_Fonts::FontDataColl font_data;
};

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  GameEngineParams params;
  params.screen_bg_color_default = Color::Black;
  params.screen_bg_color_title = Color::Black;
  params.screen_bg_color_instructions = Color::Black;
  
  Game game(argc, argv, params);

  if (argc >= 2 && strcmp(argv[1], "--help") == 0)
  {
    std::cout << "asciiroids (\"--help\" | [<frame-delay-us>])" << std::endl;
    std::cout << "  default values:" << std::endl;
    std::cout << "    <frame-delay-us>    : " << game.get_sim_delay_us() << std::endl;
    return EXIT_SUCCESS;
  }

  game.init();
  game.generate_data();
  game.run();

  return EXIT_SUCCESS;
}
