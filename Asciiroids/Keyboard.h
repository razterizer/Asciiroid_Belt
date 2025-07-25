//
//  keyboard.h
//  Asciiroids
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#pragma once
#include "Enums.h"
#include <Termin8or/Keyboard.h>
#include <Core/StringHelper.h>


Key register_keypresses(const keyboard::KeyPressDataPair& kpdp)
{
  Key curr_special_key = Key::None;
  auto key = keyboard::get_char_key(kpdp.transient);
  auto key_held = keyboard::get_char_key(kpdp.held);
  auto special_key = keyboard::get_special_key(kpdp.held);
  
  if (key == ' ')
    curr_special_key = Key::Fire;
  else if (str::to_lower(key_held) == 'a' || special_key == keyboard::SpecialKey::Left)
    curr_special_key = Key::Left;
  else if (str::to_lower(key_held) == 'd' || special_key == keyboard::SpecialKey::Right)
    curr_special_key = Key::Right;
  else if (str::to_lower(key_held) == 'w' || special_key == keyboard::SpecialKey::Up)
    curr_special_key = Key::Thrust;
  else if (str::to_lower(key_held) == 'h' || special_key == keyboard::SpecialKey::Backspace)
    curr_special_key = Key::Hyperspace;

  return curr_special_key;
}
