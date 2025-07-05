//
//  InstructionsScreen.h
//  Asciiroids
//
//  Created by Rasmus Anthin on 2025-07-05.
//

#pragma once
#include <Termin8or/ASCII_Fonts.h>


template<int NR, int NC>
void draw_instructions(ScreenHandler<NR, NC>& sh, const ASCII_Fonts::FontDataColl& font_data, const ASCII_Fonts::ColorScheme& font_colors)
{
  ASCII_Fonts::draw_text(sh, font_data, font_colors, "Pilot Episode", 1, 6, ASCII_Fonts::Font::Avatar);

  sh.write_buffer("The objective of this game is to shoot as many enemiy UFOs you can.", 7, 2, Color::White);
  sh.write_buffer("You can hide from UFOs in the clouds, just make sure that no part of the plane ", 8, 2, Color::White);
  sh.write_buffer(" sticks out from the cloud, or the enemy will find you!", 9, 2, Color::White);
  sh.write_buffer("Watch out for flocks of seagulls. If you fly through them the plane ", 10, 2, Color::White);
  sh.write_buffer(" will take damage. You can shoot the birds, but that will give minus points.", 11, 2, Color::White);
  sh.write_buffer(" We don't want to encourage bad behaviour. :)", 12, 2, Color::White);
  sh.write_buffer("Powerups can be found in the ACME cargo crates with parachutes.", 13, 2, Color::White);
  sh.write_buffer("You can see the UFOs in the radar which appear as dots around the frame:", 15, 2, Color::White);
  sh.write_buffer(" black    .       : UFO is far away and patrolling.", 16, 2, Color::White);
  sh.write_buffer(" black    o       : UFO is nearby and patrolling.", 17, 2, Color::White);
  sh.write_buffer(" blinking . and o : UFO is hunting you!", 18, 2, Color::White);
  sh.write_buffer(" dark red o       : UFO is trying to shoot at you!", 19, 2, Color::White);
  sh.write_buffer(" cyan     o       : UFO evades.", 20, 2, Color::White);

  sh.write_buffer("Controls:", 22, 2, Color::White);
  sh.write_buffer(" a,d,s,w = <,>,v,^-arrows : controls the plane.", 23, 2, Color::White);
  sh.write_buffer(" space-bar : shoot.", 24, 2, Color::White);
  sh.write_buffer(" f : fix stall. Press repeatedly to get out of stall.", 25, 2, Color::White);
  sh.write_buffer(" p : pause.", 26, 2, Color::White);
  sh.write_buffer(" q : quit.", 27, 2, Color::White);

  sh.write_buffer("Press space-bar to begin...", 29, 25, Color::White);
}

