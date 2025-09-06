//
//  keyboard.h
//  Asciiroids
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#pragma once
#include "Enums.h"
#include <Termin8or/input/Keyboard.h>
#include <Core/StringHelper.h>


Key register_keypresses(const t8::KeyPressDataPair& kpdp)
{
  Key curr_special_key = Key::None;
  auto key = t8::get_char_key(kpdp.transient);
  auto key_held = t8::get_char_key(kpdp.held);
  auto special_key = t8::get_special_key(kpdp.held);
  
  if (key == ' ')
    curr_special_key = Key::Fire;
  else if (str::to_lower(key_held) == 'a' || special_key == t8::SpecialKey::Left)
    curr_special_key = Key::Left;
  else if (str::to_lower(key_held) == 'd' || special_key == t8::SpecialKey::Right)
    curr_special_key = Key::Right;
  else if (str::to_lower(key_held) == 'w' || special_key == t8::SpecialKey::Up)
    curr_special_key = Key::Thrust;
  else if (str::to_lower(key_held) == 'h' || special_key == t8::SpecialKey::Backspace)
    curr_special_key = Key::Hyperspace;

  return curr_special_key;
}
