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
    
    sprite_spaceship = sprh.create_vector_sprite("spaceship");
    sprite_spaceship->layer_id = 2;
    sprite_spaceship->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_spaceship->add_line_segment(0, { 1, 1 }, { -1, 0 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { -1, 0 }, { 1, -1 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { 0.7f, -1.f }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 1.71f, 0 }, { 1.71f, 0 }, '*', { Color::White, Color::Transparent2 }, 2);
    sprite_spaceship->add_line_segment(1, { 1, 1 }, { -1, 0 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { -1, 0 }, { 1, -1 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 0.7f, -1.f }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->func_calc_anim_frame = [&](int sim_frame) { return spaceship_fwd_force > 0.f ? sim_frame % 2 : 0; };
    sprite_spaceship->set_rotation(0.f);
    sprite_spaceship->finalize_topology(0);
    sprite_spaceship->set_aspect_ratio(2.f);
    auto* frame = sprite_spaceship->get_curr_local_frame(0);
    frame->fill_closed_polylines = false;
    frame->fill_char = '#';
    frame->fill_style = { Color::LightGray, Color::DarkGray };
    rb_spaceship = dyn_sys.add_rigid_body(sprite_spaceship, 4.f,
      std::nullopt, {}, {},
      spaceship_rot_vel, 0.f,
      0.f, 0.f,
      crit_vel_r, crit_vel_c);
    rb_spaceship->set_orig_dir({ -1.f, 0.f });
  }
  
private:

  virtual void update() override
  {
    int anim_frame = GameEngine::get_anim_count(0);
    Key curr_game_key = register_keypresses(kpdp);
    
    //update_ship_controls(sh, src_fx_0, wave_gen, kpdp, curr_special_key,
    //                         get_sim_dt_s());
    
    // Auto-break velocities
    if (curr_game_key != Key::Left && curr_game_key != Key::Right)
      spaceship_rot_vel *= 0.5f;
    if (curr_game_key != Key::Thrust)
      spaceship_fwd_force = 0.f;
      
    switch (curr_game_key)
    {
      case Key::None:
        break;
      case Key::Left:
        spaceship_rot_vel = +1.5f;
        break;
      case Key::Right:
        spaceship_rot_vel = -1.5f;
        break;
      case Key::Thrust:
        spaceship_fwd_force = 7.f;
        break;
      case Key::Fire:
        break;
      case Key::Hyperspace:
        break;
    }
    
    // Simple Euler stepping scheme.
    auto dt = GameEngine::get_sim_dt_s();
    rb_spaceship->set_curr_ang_vel(spaceship_rot_vel);
    //spaceship_rot_ang += spaceship_rot_vel * dt;
    //sprite_spaceship->set_rotation(math::rad2deg(spaceship_rot_ang));
    spaceship_dir = rb_spaceship->get_curr_dir();
    spaceship_force = spaceship_fwd_force * spaceship_dir;
    rb_spaceship->set_curr_lin_force(spaceship_force);
    
    // Toroidal geometry update
    auto cm = rb_spaceship->get_curr_cm();
    bool warp = false;
    const int c_offs = 1;
    if (cm.r > sh.num_rows())
    {
      cm.r = 0;
      warp = true;
    }
    else if (cm.r < 0)
    {
      cm.r = sh.num_rows() - 1;
      warp = true;
    }
    else if (cm.c > sh.num_cols() + c_offs)
    {
      cm.c = -c_offs;
      warp = true;
    }
    else if (cm.c < -c_offs)
    {
      cm.c = sh.num_cols() - 1 + c_offs;
      warp = true;
    }
    if (warp)
      rb_spaceship->set_curr_cm(cm);
    
    dyn_sys.update(GameEngine::get_sim_time_s(), dt, anim_frame);
    coll_handler.update();
    
    //draw_hud(sh, ...);
    
    draw_frame(sh, Color::LightGray);
    
    if (num_lives < 0)
      num_lives = 0;
    
    if (num_lives == 0)
      GameEngine::set_state_game_over();
    
    // Game logic.
    
    // Draw stuff.
    
    if (dbg_draw_rigid_bodies)
      dyn_sys.draw_dbg(sh);
    if (dbg_draw_sprites)
      sprh.draw_dbg_pts(sh, anim_frame);
    if (dbg_draw_narrow_phase)
      coll_handler.draw_dbg_narrow_phase(sh);
    if (draw_sprites)
      sprh.draw(sh, anim_frame);
    if (dbg_draw_sprites)
      sprh.draw_dbg_bb(sh, anim_frame);
    if (dbg_draw_broad_phase)
      coll_handler.draw_dbg_broad_phase(sh, 0);
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
  
  audio::AudioSourceHandler audio;
  audio::WaveformGeneration wave_gen;
  audio::ChipTuneEngine chip_tune { audio, wave_gen };
  audio::AudioStreamSource* src_fx_shot = nullptr;
  audio::AudioStreamSource* src_fx_explosion = nullptr;
  audio::AudioStreamSource* src_fx_ufo_shot = nullptr;
  audio::AudioStreamSource* src_fx_ufo_propulsion = nullptr;
  
  std::vector<ASCII_Fonts::ColorScheme> color_schemes;
  ASCII_Fonts::FontDataColl font_data;
  
  SpriteHandler sprh;
  dynamics::DynamicsSystem dyn_sys;
  dynamics::CollisionHandler coll_handler;
  bool dbg_draw_rigid_bodies = false;
  bool dbg_draw_sprites = false;
  bool dbg_draw_narrow_phase = false;
  bool dbg_draw_broad_phase = false;
  bool draw_sprites = true;
  
  int num_lives = 3;
  
  VectorSprite* sprite_spaceship = nullptr;
  dynamics::RigidBody* rb_spaceship = nullptr;
  
  float spaceship_rot_vel = 0.f;
  //float spaceship_rot_ang = 0.f;
  float spaceship_fwd_force = 0.f;
  Vec2 spaceship_force { 0.f, 0.f };
  Vec2 spaceship_dir { -1.f, 0.f };
  float crit_vel_c = 50.f;
  float crit_vel_r = crit_vel_c/1.5f;
};

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
  GameEngineParams params;
  params.screen_bg_color_default = Color::Black;
  params.screen_bg_color_title = Color::Black;
  params.screen_bg_color_instructions = Color::Black;
  params.enable_title_screen = false;
  params.enable_instructions_screen = false;
  
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
