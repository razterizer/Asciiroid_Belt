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

#include <fstream>

// ////////////////////////////
// [x] Explosion sprites.
// [x] Spaceship collision logic (explosion + reappearance, etc).
// [ ] Score counting.
// [x] Shots should split larger asteroids into two smaller ones which travel faster than the original.
// [ ] Hyperspace.
// [ ] Large UFO.
// [ ] Small UFO.
// [ ] UFOs shoot at spaceship.
// [ ] Spaceship can shoot at UFOs.
// [ ] SFX.
// [ ] Music.
// ////////////////////////////

class Game : public GameEngine<40, 100>
{
public:
  Game(int argc, char** argv, const GameEngineParams& params)
    : GameEngine(argv[0], params)
  {
  //#ifndef _WIN32
    GameEngine::set_real_fps(15);
    GameEngine::set_sim_delay_us(50'000);
    GameEngine::set_anim_rate(0, 4); // Explosion
    GameEngine::set_anim_rate(1, 3); // Asteroids
    GameEngine::set_anim_rate(2, 5); // UFO AI
  //#endif
    if (argc >= 2 && strcmp(argv[1], "-") != 0)
      GameEngine::set_real_fps(static_cast<float>(atoi(argv[1])));
      
    if (argc >= 3 && strcmp(argv[2], "--log_mode") == 0)
    {
      if (strcmp(argv[3], "record") == 0)
        log_mode = LogMode::Record;
      else if (strcmp(argv[3], "replay") == 0)
        log_mode = LogMode::Replay;
    }
  }
  
  ~Game()
  {
    if (src_fx_shot != nullptr)
      audio.remove_source(src_fx_shot);
    if (src_fx_explosion != nullptr)
      audio.remove_source(src_fx_explosion);
    if (src_fx_ufo_shot != nullptr)
      audio.remove_source(src_fx_ufo_shot);
    if (src_fx_ufo_propulsion != nullptr)
      audio.remove_source(src_fx_ufo_propulsion);
  }

  virtual void generate_data() override
  {
    switch (log_mode)
    {
      case LogMode::None:
        break;
      case LogMode::Record:
        folder::delete_file("rec.txt");
        rec_file = std::ofstream { "rec.txt", std::ios::out | std::ios::trunc };
        rec_file << curr_rnd_seed << '\n';
        break;
      case LogMode::Replay:
        std::string log_filepath;
#ifndef _WIN32
        const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
        if (xcode_env != nullptr)
          log_filepath = "../../../../../../../../Documents/xcode/Asciiroids/Asciiroids/"; // #FIXME: Find a better solution!
#endif
        if (log_filepath.empty())
          log_filepath = "rec.txt";
        else
          log_filepath = folder::join_file_path({ log_filepath, "rec.txt" });
        rep_file = std::ifstream { log_filepath, std::ios::in };
        if (!rep_file.is_open())
        {
          std::cerr << "Error opening log file \"rec.txt\"!" << std::endl;
          exit(EXIT_FAILURE);
        }
        std::string line;
        if (std::getline(rep_file, line))
        {
          std::istringstream iss(line);
          iss >> curr_rnd_seed;
          rnd::srand(curr_rnd_seed);
        }
        //rep_file >> curr_rnd_seed;
        break;
    }
  
    try
    {
      std::string tune_path = get_exe_folder();
#ifndef _WIN32
      const char* xcode_env = std::getenv("RUNNING_FROM_XCODE");
      if (xcode_env != nullptr)
        tune_path = "../../../../../../../../Documents/xcode/Asciiroids/Asciiroids/"; // #FIXME: Find a better solution!
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
    sprite_spaceship->add_line_segment(0, { 1, -1 }, { 0.7f, -1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(0, { 1, 1 }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    
    sprite_spaceship->add_line_segment(1, { 1.71f, 0 }, { 1.71f, 0 }, '*', { Color::White, Color::Transparent2 }, 2);
    sprite_spaceship->add_line_segment(1, { 1, 1 }, { -1, 0 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { -1, 0 }, { 1, -1 }, 'o', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 0.7f, -1.f }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 1, -1 }, { 0.7f, -1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->add_line_segment(1, { 1, 1 }, { 0.7f, 1.f }, '.', { Color::White, Color::Transparent2 }, 1);
    sprite_spaceship->func_calc_anim_frame = [&](int sim_frame) { return spaceship_fwd_force > 0.f ? sim_frame % 2 : 0; };
    sprite_spaceship->set_rotation(0.f);
    sprite_spaceship->set_aspect_ratio(spaceship_ar);
    for (int frame_id = 0; frame_id < 2; ++frame_id)
    {
      sprite_spaceship->finalize_topology(frame_id);
      auto* frame = sprite_spaceship->get_curr_local_frame(frame_id);
      frame->fill_closed_polylines = false;
      frame->fill_char = '#';
      frame->fill_style = { Color::LightGray, Color::DarkGray };
    }
    rb_spaceship = dyn_sys.add_rigid_body(sprite_spaceship, 4.f,
      std::nullopt, {}, {},
      spaceship_rot_vel, 0.f,
      0.f, 0.f,
      crit_vel_r, crit_vel_c);
    rb_spaceship->set_orig_dir({ -1.f, 0.f });
    
    Color asteroid_bg_color = use_transparent_asteroids ? Color::Transparent2 : Color::Black;
    
    // +-----------+
    // |   ._  ___ |
    // |  /  \/  / |
    // | '       \ |
    // | |       / |
    // |  \___,-'  |
    // +--------+--+
    // |  /\/"/ |
    // | (    \ |
    // |  \___/ |
    // +------+-+
    // | ,^.^ |
    // | (__< |
    // +------+
    sprite_asteroid_0_big = sprh.create_bitmap_sprite("asteroid 0 big");
    sprite_asteroid_0_big->layer_id = 1;
    sprite_asteroid_0_big->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_0_big->init(5, 9);
    sprite_asteroid_0_big->create_frame(0);
    sprite_asteroid_0_big->set_sprite_chars_from_strings(0,
        "  ._  ___",
        " /  \\/  /",
        "'       \\",
        "|       /",
        " \\___,-' "
      );
    sprite_asteroid_0_big->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_0_big->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_0_big->fill_sprite_materials(0, 1);
    sprite_asteroid_0_big->enabled = false;
    
    sprite_asteroid_0_small = sprh.create_bitmap_sprite("asteroid 0 small");
    sprite_asteroid_0_small->layer_id = 1;
    sprite_asteroid_0_small->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_0_small->init(3, 6);
    sprite_asteroid_0_small->create_frame(0);
    sprite_asteroid_0_small->set_sprite_chars_from_strings(0,
        " /\\/\"/",
        "(    \\",
        " \\___/"
      );
    sprite_asteroid_0_small->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_0_small->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_0_small->fill_sprite_materials(0, 1);
    sprite_asteroid_0_small->enabled = false;
    
    sprite_asteroid_0_tiny = sprh.create_bitmap_sprite("asteroid 0 tiny");
    sprite_asteroid_0_tiny->layer_id = 1;
    sprite_asteroid_0_tiny->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_0_tiny->init(2, 4);
    sprite_asteroid_0_tiny->create_frame(0);
    sprite_asteroid_0_tiny->set_sprite_chars_from_strings(0,
        ",^.^",
        "(__<"
      );
    sprite_asteroid_0_tiny->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_0_tiny->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_0_tiny->fill_sprite_materials(0, 1);
    sprite_asteroid_0_tiny->enabled = false;
    
    // +-----------+
    // |  ,_____   |
    // | /      \  |
    // | \       | |
    // | ,  _    | |
    // | \./ |__/  |
    // +--------+--+
    // |  ,--.  |
    // | (    \ |
    // | /_/|_/ |
    // +------+-+
    // | ,-.  |
    // | Z,_) |
    // +------+
    sprite_asteroid_1_big = sprh.create_bitmap_sprite("asteroid 1 big");
    sprite_asteroid_1_big->layer_id = 1;
    sprite_asteroid_1_big->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_1_big->init(5, 9);
    sprite_asteroid_1_big->create_frame(0);
    sprite_asteroid_1_big->set_sprite_chars_from_strings(0,
        " ,_____  ",
        "/      \\ ",
        "\\       |",
        ",  _    |",
        "\\./ |__/ "
      );
    sprite_asteroid_1_big->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_1_big->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_1_big->fill_sprite_materials(0, 1);
    sprite_asteroid_1_big->enabled = false;
    
    sprite_asteroid_1_small = sprh.create_bitmap_sprite("asteroid 1 small");
    sprite_asteroid_1_small->layer_id = 1;
    sprite_asteroid_1_small->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_1_small->init(3, 6);
    sprite_asteroid_1_small->create_frame(0);
    sprite_asteroid_1_small->set_sprite_chars_from_strings(0,
        " ,--. ",
        "(    \\",
        "/_/|_/"
      );
    sprite_asteroid_1_small->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_1_small->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_1_small->fill_sprite_materials(0, 1);
    sprite_asteroid_1_small->enabled = false;
    
    sprite_asteroid_1_tiny = sprh.create_bitmap_sprite("asteroid 1 tiny");
    sprite_asteroid_1_tiny->layer_id = 1;
    sprite_asteroid_1_tiny->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_1_tiny->init(2, 4);
    sprite_asteroid_1_tiny->create_frame(0);
    sprite_asteroid_1_tiny->set_sprite_chars_from_strings(0,
        ",-. ",
        "Z,_)"
      );
    sprite_asteroid_1_tiny->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_1_tiny->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_1_tiny->fill_sprite_materials(0, 1);
    sprite_asteroid_1_tiny->enabled = false;
    
    // +-----------+
    // |  -._ _.-  |
    // | /   "  _\ |
    // | \      \  |
    // | /  .-._ / |
    // | \_/    '  |
    // +--------+--+
    // |  -.-.  |
    // | (    / |
    // | \_\._/ |
    // +------+-+
    // | ,v.  |
    // | V',) |
    // +------+
    sprite_asteroid_2_big = sprh.create_bitmap_sprite("asteroid 2 big");
    sprite_asteroid_2_big->layer_id = 1;
    sprite_asteroid_2_big->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_2_big->init(5, 9);
    sprite_asteroid_2_big->create_frame(0);
    sprite_asteroid_2_big->set_sprite_chars_from_strings(0,
        " -._ _.- ",
        "/   \"  _\\",
        "\\      \\ ",
        "/  .-._ /",
        "\\_/    ' "
      );
    sprite_asteroid_2_big->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_2_big->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_2_big->fill_sprite_materials(0, 1);
    sprite_asteroid_2_big->enabled = false;
    
    sprite_asteroid_2_small = sprh.create_bitmap_sprite("asteroid 2 small");
    sprite_asteroid_2_small->layer_id = 1;
    sprite_asteroid_2_small->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_2_small->init(3, 6);
    sprite_asteroid_2_small->create_frame(0);
    sprite_asteroid_2_small->set_sprite_chars_from_strings(0,
        " -.-. ",
        "(    /",
        "\\_\\._/"
      );
    sprite_asteroid_2_small->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_2_small->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_2_small->fill_sprite_materials(0, 1);
    sprite_asteroid_2_small->enabled = false;
    
    sprite_asteroid_2_tiny = sprh.create_bitmap_sprite("asteroid 2 tiny");
    sprite_asteroid_2_tiny->layer_id = 1;
    sprite_asteroid_2_tiny->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_asteroid_2_tiny->init(2, 4);
    sprite_asteroid_2_tiny->create_frame(0);
    sprite_asteroid_2_tiny->set_sprite_chars_from_strings(0,
        ",v. ",
        "V',)"
      );
    sprite_asteroid_2_tiny->fill_sprite_fg_colors(0, Color::White);
    sprite_asteroid_2_tiny->fill_sprite_bg_colors(0, asteroid_bg_color);
    sprite_asteroid_2_tiny->fill_sprite_materials(0, 1);
    sprite_asteroid_2_tiny->enabled = false;
    
    generate_big_asteroids(4);
    
    sprite_explosion = sprh.create_bitmap_sprite("explosion");
    sprite_explosion->layer_id = 3;
    sprite_explosion->pos = { sh.num_rows()/2, sh.num_cols()/2 };
    sprite_explosion->init(5, 9);
    sprite_explosion->enabled = false;
    sprite_explosion->create_frame(0);
    sprite_explosion->set_sprite_chars_from_strings(0,
        "         ",
        "         ",
        "    .    ",
        "         ",
        "         "
      );
    sprite_explosion->fill_sprite_fg_colors(0, Color::White);
    sprite_explosion->fill_sprite_bg_colors(0, Color::Transparent2);
    sprite_explosion->create_frame(1);
    sprite_explosion->set_sprite_chars_from_strings(1,
        "         ",
        "    ..   ",
        "   . ..  ",
        "    .    ",
        "         "
      );
    sprite_explosion->fill_sprite_fg_colors(1, Color::White);
    sprite_explosion->fill_sprite_bg_colors(1, Color::Transparent2);
    sprite_explosion->create_frame(2);
    sprite_explosion->set_sprite_chars_from_strings(2,
        "    . .  ",
        "  .  . . ",
        " . .  .  ",
        "  .    . ",
        "    .    "
      );
    sprite_explosion->fill_sprite_fg_colors(2, Color::White);
    sprite_explosion->fill_sprite_bg_colors(2, Color::Transparent2);
    sprite_explosion->create_frame(3);
    sprite_explosion->set_sprite_chars_from_strings(3,
        " . .. .  ",
        "  .  . . ",
        " ..    . ",
        "  .   .  ",
        " .  .   ."
      );
    sprite_explosion->fill_sprite_fg_colors(3, Color::White);
    sprite_explosion->fill_sprite_bg_colors(3, Color::Transparent2);
    sprite_explosion->create_frame(4);
    sprite_explosion->set_sprite_chars_from_strings(4,
        ".   .  . ",
        " .     . ",
        ".       .",
        "         ",
        " .  .   ."
      );
    sprite_explosion->fill_sprite_fg_colors(4, Color::White);
    sprite_explosion->fill_sprite_bg_colors(4, Color::Transparent2);
    sprite_explosion->create_frame(5);
    sprite_explosion->set_sprite_chars_from_strings(5,
        "         ",
        "         ",
        ".        ",
        "         ",
        "     .   "
      );
    sprite_explosion->fill_sprite_fg_colors(5, Color::White);
    sprite_explosion->fill_sprite_bg_colors(5, Color::Transparent2);
    sprite_explosion->create_frame(6);
    sprite_explosion->set_sprite_chars_from_strings(6,
        "         ",
        "         ",
        "         ",
        "         ",
        "         "
      );
    sprite_explosion->fill_sprite_fg_colors(6, Color::White);
    sprite_explosion->fill_sprite_bg_colors(6, Color::Transparent2);
  }
  
private:

  template<int NR, int NC>
  bool toroidal_wrap(const ScreenHandler<NR, NC>& sh, RC& pos, int c_offs, int r_offs)
  {
    if (pos.r > sh.num_rows() + r_offs)
    {
      pos.r = -r_offs;
      return true;
    }
    else if (pos.r < -r_offs)
    {
      pos.r = sh.num_rows() - 1 + r_offs;
      return true;
    }
    else if (pos.c > sh.num_cols() + c_offs)
    {
      pos.c = -c_offs;
      return true;
    }
    else if (pos.c < -c_offs)
    {
      pos.c = sh.num_cols() - 1 + c_offs;
      return true;
    }
    return false;
  }
  
  void cleanup_asteroids()
  {
    for (auto& asteroid : asteroids_vec)
    {
      sprh.remove_sprite(asteroid.sprite);
      dyn_sys.remove_rigid_body(asteroid.rb);
    }
    asteroids_vec.clear();
  }
  
  void generate_big_asteroids(int num_asteroids)
  {
    for (int a_idx = 0; a_idx < num_asteroids; ++a_idx)
    {
      auto& asteroid = asteroids_vec.emplace_back();
      std::string sprite_src_name = "asteroid " + std::to_string(rnd::rand_int(0, 2)) + " big";
      asteroid.sprite = static_cast<BitmapSprite*>(sprh.clone_sprite("asteroid big id:" + std::to_string(global_asteroid_id++), sprite_src_name));
      //std::cout << asteroid.sprite->get_name() << " : " << sprite_src_name << std::endl;
      asteroid.sprite->enabled = true;
      auto nr_f = static_cast<float>(sh.num_rows());
      auto nc_f = static_cast<float>(sh.num_cols());
      Vec2 pos;
      for (;;)
      {
        pos = Vec2 { rnd::rand_float(0.f, nr_f), rnd::rand_float(0.f, nc_f) };
        if (math::distance_squared_ar(pos, rb_spaceship->get_curr_cm(), 2.f) > c_min_ship_asteroid_dist_sq)
          break;
      }
      asteroid.rb = dyn_sys.add_rigid_body(asteroid.sprite, 20.f, // mass
        pos, // pos
        Vec2 { rnd::randn(0.f, 3.f), rnd::randn(0.f, 3.f) } // vel
      );
    }
    coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
    coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ast");
  }
  
  template<int NR, int NC>
  void draw_hud(ScreenHandler<NR, NC>& sh)
  {
    sh.write_buffer(str::adjust_str(std::to_string(score), str::Adjustment::Right, 6), 1, 8, Color::White);
  
    for (int life_idx = 0; life_idx < std::min(num_lives, 10); ++life_idx)
    {
      sh.write_buffer("A", 2, 10 + life_idx, Color::White);
    }
  }
  
  // ///////

  virtual void update() override
  {
    // Game logic.
    bool log_finished = false;
    int anim_frame = GameEngine::get_anim_count(0);
    auto t = GameEngine::get_sim_time_s();
    Key curr_game_key = Key::None;
    if (log_mode == LogMode::Replay)
    {
      std::string line;
      if (std::getline(rep_file, line))
      {
        std::istringstream iss(line);
        int log_frame_count = 0;
        iss >> log_frame_count;
        if (log_frame_count != GameEngine::get_frame_count())
        {
          std::cerr << "REPLAY ERROR : Expected frame number " << GameEngine::get_frame_count() << " but received frame number " << log_frame_count << ". Exiting!" << std::endl;
          exit(EXIT_FAILURE);
        }
        char log_key = ' ';
        iss >> log_key;
        if (log_key == 'L')
          curr_game_key = Key::Left;
        else if (log_key == 'R')
          curr_game_key = Key::Right;
        else if (log_key == 'T')
          curr_game_key = Key::Thrust;
        else if (log_key == 'F')
          curr_game_key = Key::Fire;
        else if (log_key == 'H')
          curr_game_key = Key::Hyperspace;
        else
          curr_game_key = Key::None;
      }
      else
        log_finished = true;
    }
    else
      curr_game_key = register_keypresses(kpdp);
    
    //update_ship_controls(sh, src_fx_0, wave_gen, kpdp, curr_special_key,
    //                         get_sim_dt_s());
    
    // Auto-break velocities
    if (curr_game_key != Key::Left && curr_game_key != Key::Right)
      spaceship_rot_vel *= 0.5f;
    if (curr_game_key != Key::Thrust)
      spaceship_fwd_force = 0.f;
      
    if (log_mode == LogMode::Record)
      rec_file << std::to_string(GameEngine::get_frame_count()) << ' ';
    if (sprite_spaceship->enabled)
    {
      switch (curr_game_key)
      {
        case Key::None:
          break;
        case Key::Left:
          if (log_mode == LogMode::Record)
            rec_file << 'L';
          spaceship_rot_vel = +1.5f;
          break;
        case Key::Right:
          if (log_mode == LogMode::Record)
            rec_file << 'R';
          spaceship_rot_vel = -1.5f;
          break;
        case Key::Thrust:
          if (log_mode == LogMode::Record)
            rec_file << 'T';
          spaceship_fwd_force = 10.f; //7.f;
          break;
        case Key::Fire:
          if (log_mode == LogMode::Record)
            rec_file << 'F';
          if (t - shot_timestamp > shot_min_time_interval)
          {
            Shot shot;
            shot.dir = Vec2 { spaceship_dir.r / spaceship_ar, spaceship_dir.c };
            shot.dir = math::normalize(shot.dir);
            shot.pos = rb_spaceship->get_curr_cm() + spaceship_dir * 1.f;
            shot.time_0 = GameEngine::get_sim_time_s();
            shots_vec.emplace_back(shot);
            shot_timestamp = t;
          }
          break;
        case Key::Hyperspace:
          if (log_mode == LogMode::Record)
            rec_file << 'H';
          break;
      }
    }
    if (log_mode == LogMode::Record)
      rec_file << '\n';
    rec_file.flush();
    
    // Simple Euler stepping scheme.
    auto dt = GameEngine::get_sim_dt_s();
    rb_spaceship->set_curr_ang_vel(spaceship_rot_vel);
    spaceship_dir = rb_spaceship->get_curr_dir();
    spaceship_force = spaceship_fwd_force * spaceship_dir;
    rb_spaceship->set_curr_lin_force(spaceship_force);
    
    stlutils::erase_if(shots_vec, [&](const Shot& shot) {
      return t - shot.time_0 > shot_lifetime || shot.hit;
    });
    for (auto& shot : shots_vec)
      shot.pos += shot.dir * shot_speed * dt;
    
    // Toroidal geometry update
    auto cm = to_RC_round(rb_spaceship->get_curr_cm());
    if (toroidal_wrap(sh, cm, 0, 1))
      rb_spaceship->set_curr_cm(to_Vec2(cm));
    for (auto& shot : shots_vec)
    {
      auto pos = to_RC_round(shot.pos);
      if (toroidal_wrap(sh, pos, 0, 0))
        shot.pos = to_Vec2(pos);
    }
    for (auto& asteroid : asteroids_vec)
    {
      auto a_cm = to_RC_round(asteroid.rb->get_curr_cm());
      if (toroidal_wrap(sh, a_cm, 0, 0))
        asteroid.rb->set_curr_cm(to_Vec2(a_cm));
    }
    
    // Handle collisions and generate explosions:
    //  * asteroid <-> spaceship
    //  * small ufo <-> spaceship
    //  * large ufo <-> spaceship
    auto f_generate_explosion = [&](const RC& pos) -> ExplosionData*
    {
      auto& explosion = explosions_vec.emplace_back(std::make_unique<ExplosionData>());
      explosion->trig = true;
      explosion->sprite = static_cast<BitmapSprite*>(sprh.clone_sprite("explosion " + std::to_string(global_explosion_id++), "explosion"));
      explosion->sprite->enabled = true;
      explosion->sprite->pos = pos - sprite_explosion->get_size() / 2;
      auto* expl_raw_ptr = explosion.get();
      explosion->sprite->func_calc_anim_frame = [expl_raw_ptr](int sim_frame) { return expl_raw_ptr->anim_ctr; };
      return expl_raw_ptr;
    };
    auto isect_data = coll_handler.get_isect_world_positions();
    for (const auto& id : isect_data)
    {
      if (!stlutils::contains_if(explosions_vec, [&id](const auto& expl) { return (id.node_A == expl->isect_data.node_A && id.node_B == expl->isect_data.node_B) || (id.node_A == expl->isect_data.node_B && id.node_B == expl->isect_data.node_A); }))
      {
        auto* explosion = f_generate_explosion(to_RC_round(id.world_pos));
        explosion->isect_data = id;
      }
    }

    // Split asteroids when shot upon.
    new_asteroids_vec.clear();
    for (auto& asteroid : asteroids_vec)
    {
      auto aabb_asteroid = asteroid.sprite->calc_curr_AABB(0);
      RC hit_rc;
      for (auto& shot : shots_vec)
      {
        auto shot_rc = to_RC_round(shot.pos);
        if (aabb_asteroid.contains(shot_rc))
        {
          asteroid.hit = true;
          shot.hit = true;
          hit_rc = shot_rc;
          break;
        }
      }
      stlutils::erase_if(shots_vec, [](const auto& s) { return s.hit; });
      if (asteroid.hit)
      {
        f_generate_explosion(hit_rc);
        
        if (asteroid.level <= 1)
        {
          Asteroid a_child[2];
          Vec2 a_child_dir[2];
          const float ar = 1.8f;
          a_child_dir[0] = Vec2 { rnd::rand_float(-1.f, +1.f)/ar, rnd::rand_float(-1.f, +1.f) };
          a_child_dir[0] = math::normalize(a_child_dir[0]);
          a_child_dir[1] = Vec2 {
            -a_child_dir[0].r + rnd::randn(0.f, 0.2f)/ar,
            -a_child_dir[0].c + rnd::randn(0.f, 0.2f)
          };
          a_child_dir[1] = math::normalize(a_child_dir[1]);
          for (int a_idx = 0; a_idx < 2; ++a_idx)
          {
            a_child[a_idx].level = asteroid.level + 1;
            std::string a_size = a_child[a_idx].level == 1 ? "small" : "tiny";
            std::string sprite_src_name = "asteroid " + std::to_string(rnd::rand_int(0, 2)) + " " + a_size;
            a_child[a_idx].sprite = static_cast<BitmapSprite*>(sprh.clone_sprite("asteroid " + a_size + " id:" + std::to_string(global_asteroid_id++), sprite_src_name));
            //std::cout << a_child[a_idx].sprite->get_name() << " : " << sprite_src_name << std::endl;
            a_child[a_idx].sprite->enabled = true;
            auto pos = asteroid.rb->get_curr_cm();
            pos.r += a_child_dir[a_idx].r * rnd::rand_float(0.1f, 3.f)/ar;
            pos.c += a_child_dir[a_idx].c * rnd::rand_float(0.1f, 3.f);
            auto vel = a_child_dir[a_idx] * (math::length(asteroid.rb->get_curr_lin_vel()) + rnd::rand_float(0.05f, 3.f));
            a_child[a_idx].rb = dyn_sys.add_rigid_body(a_child[a_idx].sprite, 20.f, // mass
              pos, vel);
            new_asteroids_vec.emplace_back(a_child[a_idx]);
          }
        }
        
        sprh.remove_sprite(asteroid.sprite);
        dyn_sys.remove_rigid_body(asteroid.rb);
        switch (asteroid.level)
        {
          case 0: score += 20; break;
          case 1: score += 50; break;
          case 2: score += 100; break;
        }
      }
    }
    stlutils::erase_if(asteroids_vec, [](const auto& a) { return a.hit; });
    stlutils::append(asteroids_vec, new_asteroids_vec);
    if (!new_asteroids_vec.empty())
    {
      coll_handler.rebuild_BVH(sh.num_rows(), sh.num_cols(), &dyn_sys);
      coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "ast");
    }
    
    // If spaceship collided with something then lose a life and make the ship disappear and reappear in a safe zone.
    auto id_it = stlutils::find_if(isect_data, [&](const auto& id) { return id.node_A->rigid_body == rb_spaceship || id.node_B->rigid_body == rb_spaceship; });
    if (id_it != isect_data.end())
    {
      sprite_spaceship->enabled = false;
      coll_handler.exclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa");
      spaceship_explosion = true;
      spaceship_killed_timestamp = t;
      num_lives--;
    }
    if (spaceship_explosion && t - spaceship_killed_timestamp > 0.2f)
    {
      const auto& spaceship_pos = rb_spaceship->get_curr_cm();
      if (!stlutils::contains_if(asteroids_vec, [&spaceship_pos, this](const auto& a) { return math::distance_squared_ar(a.rb->get_curr_cm(), spaceship_pos, 2.f) < c_min_ship_asteroid_dist_sq; }))
      {
        spaceship_explosion = false;
        sprite_spaceship->enabled = true;
        rb_spaceship->set_curr_cm({ sh.num_rows()/2.f, sh.num_cols()/2.f });
        rb_spaceship->set_curr_lin_vel({ 0, 0 });
        rb_spaceship->set_curr_ang(0.f);
        coll_handler.reinclude_all_rigid_bodies_of_prefixes(&dyn_sys, "ast", "spa");
      }
    }
    
    for (auto& explosion : explosions_vec)
    {
      if (explosion->trig)
      {
        explosion->timestamp = GameEngine::get_anim_count(0);
        explosion->anim_ctr = 0;
        explosion->trig = false;
      }
      else if (explosion->anim_ctr < 6)
        explosion->anim_ctr = GameEngine::get_anim_count(0) - explosion->timestamp;
      else
        explosion->sprite->enabled = false;
    }
    stlutils::erase_if(explosions_vec, [&](auto& expl)
    {
      if (!expl->sprite->enabled)
      {
        sprh.remove_sprite(expl->sprite);
        return true;
      }
      return false;
    });
    
    if (!level_up && asteroids_vec.empty())
    {
      level_timestamp = t;
      cleanup_asteroids();
      level_up = true;
    }
    else if (level_up && t - level_timestamp > 2.f)
    {
      level++;
      generate_big_asteroids(level*2);
      level_up = false;
    }
    
    if (num_lives < 0)
      num_lives = 0;
    
    if (num_lives == 0)
      GameEngine::set_state_game_over();
      
    // Update dynamics and collisions.
    dyn_sys.update(GameEngine::get_sim_time_s(), dt, anim_frame);
    coll_handler.update();
    
    // Draw stuff.
    
    draw_hud(sh);
    
    draw_frame(sh, Color::LightGray);
    
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
      
    for (const auto& shot : shots_vec)
      sh.write_buffer(".", math::roundI(shot.pos.r), math::roundI(shot.pos.c), Color::White);
    
    if (log_finished)
      exit(EXIT_SUCCESS);
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
  
  int num_lives = 3; // max visible lives : 10, but more lives can be stored. You gain an extra life for every 10'000 points gained.
  int score = 0; // max score : 999990
  int level = 2;
  float level_timestamp = 0.f;
  bool level_up = false;
  const float c_min_ship_asteroid_dist_sq = math::sq(20.f);
  
  VectorSprite* sprite_spaceship = nullptr;
  dynamics::RigidBody* rb_spaceship = nullptr;
  
  const float spaceship_ar = 2.f;
  float spaceship_rot_vel = 0.f;
  float spaceship_fwd_force = 0.f;
  Vec2 spaceship_force { 0.f, 0.f };
  Vec2 spaceship_dir { -1.f, 0.f };
  float crit_vel_c = 30.f;
  float crit_vel_r = crit_vel_c/1.5f;
  bool spaceship_explosion = false;
  float spaceship_killed_timestamp = 0.f;
  
  float shot_speed = 31.f;
  float shot_lifetime = 2.f;
  float shot_min_time_interval = 0.1f; // Minimum time allowed between shots.
  float shot_timestamp = 0.f;
  struct Shot
  {
    Vec2 dir;
    Vec2 pos;
    bool hit = false;
    float time_0 = 0.f;
  };
  std::vector<Shot> shots_vec;
  
  bool use_transparent_asteroids = true;
  
  BitmapSprite* sprite_asteroid_0_big = nullptr;
  BitmapSprite* sprite_asteroid_1_big = nullptr;
  BitmapSprite* sprite_asteroid_2_big = nullptr;
  BitmapSprite* sprite_asteroid_0_small = nullptr;
  BitmapSprite* sprite_asteroid_1_small = nullptr;
  BitmapSprite* sprite_asteroid_2_small = nullptr;
  BitmapSprite* sprite_asteroid_0_tiny = nullptr;
  BitmapSprite* sprite_asteroid_1_tiny = nullptr;
  BitmapSprite* sprite_asteroid_2_tiny = nullptr;
  
  struct Asteroid
  {
    BitmapSprite* sprite = nullptr;
    dynamics::RigidBody* rb = nullptr;
    int level = 0; // 0 : big, 1 : small, 2 : tiny.
    bool hit = false;
  };
  int global_asteroid_id = 0;
  std::vector<Asteroid> asteroids_vec;
  std::vector<Asteroid> new_asteroids_vec;
  
  BitmapSprite* sprite_explosion = nullptr;
  //VectorSprite* sprite_broken_ship = nullptr;
  
  struct ExplosionData
  {
    bool trig = false;
    int timestamp = 0;
    int anim_ctr = 0;
    dynamics::CollisionHandler::IsectData isect_data;
    BitmapSprite* sprite = nullptr;
  };
  int global_explosion_id = 0;
  std::vector<std::unique_ptr<ExplosionData>> explosions_vec;
  
  std::ofstream rec_file;
  std::ifstream rep_file;
  enum class LogMode { None, Record, Replay } log_mode = LogMode::None;
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
    std::cout << "asciiroids (\"--help\" | [(<frame-delay-us> | '-') [--log_mode (record | replay)]])" << std::endl;
    std::cout << "  default values:" << std::endl;
    std::cout << "    <frame-delay-us>    : " << game.get_sim_delay_us() << std::endl;
    return EXIT_SUCCESS;
  }

  game.init();
  game.generate_data();
  game.run();

  return EXIT_SUCCESS;
}
