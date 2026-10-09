/*
TAG_SWAP_CODES.C

Written by tools/tag_swap_codes.py: do not edit. Where the integers and
reals are in each structure a tag schema names (tag_byte_swap.c).
*/

#include "cseries.h"
#include "tag_byte_swap.h"

static struct tag_swap_run const byte_runs[] = {{0, 0, 0}};
static struct tag_swap_run const byte_rectangle3d_runs[] = {{0, 0, 0}};
static struct tag_swap_run const long_runs[] = {{0, 4, 1}};
static struct tag_swap_run const real_runs[] = {{0, 4, 1}};
static struct tag_swap_run const real_matrix3x3_runs[] = {{0, 4, 9}};
static struct tag_swap_run const real_plane3d_runs[] = {{0, 4, 4}};
static struct tag_swap_run const real_point2d_runs[] = {{0, 4, 2}};
static struct tag_swap_run const real_point3d_runs[] = {{0, 4, 3}};
static struct tag_swap_run const real_vector4d_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct__biped_definition_runs[] = {{0, 4, 7}, {28, 2, 4}, {36, 4, 116}, {500, 2, 2}, {504, 4, 3}};
static struct tag_swap_run const struct__control_definition_runs[] = {{0, 2, 2}, {4, 4, 1}, {88, 4, 12}};
static struct tag_swap_run const struct__device_definition_runs[] = {{0, 4, 7}, {28, 2, 4}, {36, 4, 60}};
static struct tag_swap_run const struct__equipment_definition_runs[] = {{0, 2, 2}, {4, 4, 5}};
static struct tag_swap_run const struct__item_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 1}, {12, 2, 2}, {16, 4, 4}, {32, 2, 4}, {40, 4, 89}};
static struct tag_swap_run const struct__machine_definition_runs[] = {{0, 2, 2}, {4, 4, 21}, {88, 2, 2}, {92, 4, 14}};
static struct tag_swap_run const struct__object_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 15}};
static struct tag_swap_run const struct__projectile_definition_runs[] = {{0, 4, 1}, {4, 2, 6}, {16, 4, 25}, {116, 2, 2}, {120, 4, 22}};
static struct tag_swap_run const struct__unit_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 5}, {28, 2, 4}, {36, 4, 2}, {108, 4, 29}, {224, 2, 2}, {228, 4, 14}, {284, 2, 2}, {288, 4, 10}, {328, 2, 4}, {336, 4, 9}};
static struct tag_swap_run const struct__weapon_definition_runs[] = {{0, 4, 1}, {36, 2, 6}, {48, 4, 40}, {208, 2, 2}, {212, 4, 8}, {244, 2, 2}, {248, 4, 56}, {472, 2, 2}, {476, 4, 9}};
static struct tag_swap_run const struct_actor_definition_runs[] = {{0, 4, 5}, {20, 2, 2}, {24, 4, 148}, {616, 2, 4}, {624, 4, 2}, {632, 2, 2}, {636, 4, 10}, {676, 2, 2}, {680, 4, 20}, {760, 2, 2}, {764, 4, 22}, {852, 2, 2}, {856, 4, 104}};
static struct tag_swap_run const struct_actor_starting_location_runs[] = {{0, 4, 4}, {16, 2, 1}, {20, 2, 4}};
static struct tag_swap_run const struct_actor_variant_change_colors_runs[] = {{0, 4, 8}};
static struct tag_swap_run const struct_actor_variant_definition_runs[] = {{0, 4, 19}, {76, 2, 2}, {80, 4, 65}, {340, 2, 2}, {344, 4, 10}, {384, 2, 4}, {392, 4, 18}, {464, 2, 2}, {468, 4, 3}, {480, 2, 2}, {484, 4, 10}, {524, 2, 2}, {528, 4, 10}};
static struct tag_swap_run const struct_ai_animation_reference_definition_runs[] = {{32, 4, 7}};
static struct tag_swap_run const struct_ai_command_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 8}, {28, 4, 1}};
static struct tag_swap_run const struct_ai_command_list_definition_runs[] = {{32, 4, 3}, {44, 2, 2}, {48, 4, 12}};
static struct tag_swap_run const struct_ai_command_point_definition_runs[] = {{0, 4, 5}};
static struct tag_swap_run const struct_ai_conversation_runs[] = {{32, 2, 2}, {36, 4, 2}, {80, 4, 9}};
static struct tag_swap_run const struct_ai_recording_reference_definition_runs[] = {{32, 4, 2}};
static struct tag_swap_run const struct_ai_script_reference_definition_runs[] = {{32, 4, 2}};
static struct tag_swap_run const struct_animation_runs[] = {{32, 2, 4}, {40, 4, 1}, {44, 2, 2}, {48, 4, 1}, {52, 2, 6}, {66, 2, 1}, {68, 4, 28}};
static struct tag_swap_run const struct_animation_graph_runs[] = {{0, 4, 25}, {100, 2, 2}, {104, 4, 6}};
static struct tag_swap_run const struct_animation_graph_animation_index_runs[] = {{0, 2, 1}};
static struct tag_swap_run const struct_animation_graph_device_animations_runs[] = {{0, 4, 24}};
static struct tag_swap_run const struct_animation_graph_ik_point_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_animation_graph_node_runs[] = {{32, 2, 4}, {40, 4, 6}};
static struct tag_swap_run const struct_animation_graph_object_overlay_runs[] = {{0, 2, 4}, {8, 4, 3}};
static struct tag_swap_run const struct_animation_graph_sound_reference_runs[] = {{0, 4, 5}};
static struct tag_swap_run const struct_animation_graph_unit_seat_runs[] = {{32, 4, 2}, {40, 2, 2}, {44, 4, 2}, {52, 2, 2}, {56, 4, 11}};
static struct tag_swap_run const struct_animation_graph_weapon_animations_runs[] = {{0, 4, 7}};
static struct tag_swap_run const struct_animation_graph_weapon_class_runs[] = {{96, 4, 2}, {104, 2, 2}, {108, 4, 2}, {116, 2, 2}, {120, 4, 17}};
static struct tag_swap_run const struct_animation_graph_weapon_type_runs[] = {{32, 4, 7}};
static struct tag_swap_run const struct_antenna_definition_runs[] = {{32, 4, 44}};
static struct tag_swap_run const struct_antenna_vertex_definition_runs[] = {{0, 4, 10}, {40, 2, 2}, {44, 4, 21}};
static struct tag_swap_run const struct_auxilary_meter_definition_runs[] = {{0, 2, 2}, {4, 4, 4}, {20, 2, 2}, {24, 4, 2}, {32, 2, 2}, {36, 4, 13}, {88, 2, 2}, {92, 4, 2}, {100, 2, 4}, {108, 4, 4}, {124, 2, 2}, {128, 4, 2}, {136, 2, 2}, {140, 4, 13}, {194, 2, 1}, {198, 2, 1}, {200, 4, 31}};
static struct tag_swap_run const struct_auxilary_overlay_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 13}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 4}, {104, 2, 2}, {108, 4, 6}};
static struct tag_swap_run const struct_biped_contact_point_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_biped_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 16}, {384, 2, 2}, {388, 4, 5}, {408, 2, 4}, {416, 4, 2}, {488, 4, 29}, {604, 2, 2}, {608, 4, 14}, {664, 2, 2}, {668, 4, 10}, {708, 2, 4}, {716, 4, 16}, {780, 2, 4}, {788, 4, 116}, {1252, 2, 2}, {1256, 4, 3}};
static struct tag_swap_run const struct_bitmap_data_runs[] = {{0, 4, 1}, {4, 2, 10}, {24, 4, 6}};
static struct tag_swap_run const struct_bitmap_group_runs[] = {{0, 2, 4}, {8, 4, 3}, {20, 2, 4}, {28, 4, 12}, {76, 2, 4}, {84, 4, 6}};
static struct tag_swap_run const struct_bitmap_group_sequence_runs[] = {{32, 2, 2}, {36, 4, 7}};
static struct tag_swap_run const struct_bitmap_group_sprite_runs[] = {{0, 2, 2}, {4, 4, 7}};
static struct tag_swap_run const struct_breakable_surface_particle_effect_runs[] = {{0, 4, 32}};
static struct tag_swap_run const struct_bsp2d_node_runs[] = {{0, 4, 5}};
static struct tag_swap_run const struct_bsp2d_reference_runs[] = {{0, 4, 2}};
static struct tag_swap_run const struct_bsp3d_node_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_camera_track_control_point_runs[] = {{0, 4, 15}};
static struct tag_swap_run const struct_camera_track_definition_runs[] = {{0, 4, 12}};
static struct tag_swap_run const struct_collision_bsp_runs[] = {{0, 4, 24}};
static struct tag_swap_run const struct_collision_edge_runs[] = {{0, 4, 6}};
static struct tag_swap_run const struct_collision_leaf_runs[] = {{0, 2, 2}, {4, 4, 1}};
static struct tag_swap_run const struct_collision_model_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 50}, {208, 2, 2}, {212, 4, 6}, {236, 2, 2}, {240, 4, 106}};
static struct tag_swap_run const struct_collision_node_runs[] = {{32, 2, 4}, {40, 4, 2}, {48, 2, 2}, {52, 4, 3}};
static struct tag_swap_run const struct_collision_surface_runs[] = {{0, 4, 2}, {10, 2, 1}};
static struct tag_swap_run const struct_collision_vertex_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_color_table_color_runs[] = {{32, 4, 4}};
static struct tag_swap_run const struct_color_table_definition_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_continuous_damage_effect_definition_runs[] = {{0, 4, 22}, {88, 2, 2}, {92, 4, 50}, {292, 2, 2}, {296, 4, 54}};
static struct tag_swap_run const struct_contrail_definition_runs[] = {{0, 2, 2}, {4, 4, 5}, {24, 2, 2}, {28, 4, 9}, {64, 2, 2}, {68, 4, 16}, {132, 2, 2}, {136, 4, 7}, {164, 2, 4}, {176, 2, 1}, {312, 4, 3}};
static struct tag_swap_run const struct_contrail_point_state_runs[] = {{0, 4, 26}};
static struct tag_swap_run const struct_control_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 22}, {408, 2, 4}, {416, 4, 60}, {656, 2, 2}, {660, 4, 1}, {744, 4, 12}};
static struct tag_swap_run const struct_control_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}, {40, 2, 2}, {44, 4, 1}, {48, 2, 3}};
static struct tag_swap_run const struct_damage_definition_runs[] = {{0, 2, 2}, {4, 4, 54}};
static struct tag_swap_run const struct_damage_effect_definition_runs[] = {{0, 4, 9}, {36, 2, 2}, {40, 4, 4}, {56, 2, 2}, {60, 4, 10}, {100, 2, 2}, {104, 4, 4}, {120, 2, 2}, {124, 4, 8}, {156, 2, 2}, {160, 4, 12}, {208, 2, 2}, {212, 4, 5}, {232, 2, 2}, {236, 4, 54}, {452, 2, 2}, {456, 4, 54}};
static struct tag_swap_run const struct_damage_permutation_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_damage_region_runs[] = {{32, 4, 13}};
static struct tag_swap_run const struct_damage_resistance_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 50}, {208, 2, 2}, {212, 4, 6}, {236, 2, 2}, {240, 4, 90}};
static struct tag_swap_run const struct_damage_resistance_material_runs[] = {{32, 4, 1}, {36, 2, 2}, {40, 4, 8}};
static struct tag_swap_run const struct_decal_definition_runs[] = {{0, 2, 4}, {8, 4, 20}, {88, 2, 2}, {92, 4, 14}, {148, 2, 2}, {152, 4, 7}, {180, 2, 8}, {196, 4, 15}, {256, 2, 2}, {260, 4, 2}};
static struct tag_swap_run const struct_detail_object_runs[] = {{4, 2, 1}};
static struct tag_swap_run const struct_detail_object_cell_definition_runs[] = {{0, 2, 4}, {8, 4, 6}};
static struct tag_swap_run const struct_detail_object_collection_definition_runs[] = {{0, 2, 2}, {4, 4, 31}};
static struct tag_swap_run const struct_detail_object_type_definition_runs[] = {{36, 4, 7}};
static struct tag_swap_run const struct_device_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 22}, {408, 2, 4}, {416, 4, 60}};
static struct tag_swap_run const struct_dialogue_definition_runs[] = {{0, 2, 2}, {4, 4, 1027}};
static struct tag_swap_run const struct_effect_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 14}};
static struct tag_swap_run const struct_effect_event_definition_runs[] = {{0, 4, 17}};
static struct tag_swap_run const struct_effect_location_definition_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_effect_part_definition_runs[] = {{0, 2, 4}, {8, 4, 24}};
static struct tag_swap_run const struct_effect_particles_definition_runs[] = {{0, 2, 6}, {12, 4, 23}, {104, 2, 4}, {112, 4, 30}};
static struct tag_swap_run const struct_encounter_definition_runs[] = {{32, 4, 1}, {36, 2, 4}, {44, 4, 20}, {124, 2, 2}, {128, 4, 12}};
static struct tag_swap_run const struct_encounter_player_starting_location_runs[] = {{0, 4, 4}, {16, 2, 6}};
static struct tag_swap_run const struct_equipment_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 16}, {384, 2, 2}, {388, 4, 1}, {392, 2, 2}, {396, 4, 4}, {412, 2, 4}, {420, 4, 89}, {776, 2, 2}, {780, 4, 5}};
static struct tag_swap_run const struct_equipment_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}};
static struct tag_swap_run const struct_firing_position_definition_runs[] = {{0, 4, 3}, {12, 2, 2}, {20, 4, 1}};
static struct tag_swap_run const struct_flag_attachment_point_runs[] = {{0, 2, 2}, {4, 4, 4}};
static struct tag_swap_run const struct_flag_definition_runs[] = {{0, 4, 1}, {4, 2, 6}, {16, 4, 20}};
static struct tag_swap_run const struct_fog_definition_full_runs[] = {{0, 2, 2}, {4, 4, 1}, {88, 4, 1}, {96, 4, 1}, {104, 4, 1}, {116, 4, 4}, {132, 2, 2}, {136, 4, 5}, {160, 4, 4}, {184, 4, 13}, {244, 4, 8}};
static struct tag_swap_run const struct_font_character_runs[] = {{0, 2, 8}, {16, 4, 1}};
static struct tag_swap_run const struct_font_character_index_runs[] = {{0, 2, 1}};
static struct tag_swap_run const struct_font_character_table_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_font_header_runs[] = {{0, 4, 1}, {4, 2, 4}, {12, 4, 36}};
static struct tag_swap_run const struct_game_globals_runs[] = {{0, 4, 107}};
static struct tag_swap_run const struct_game_globals_difficulty_information_runs[] = {{0, 4, 161}};
static struct tag_swap_run const struct_game_globals_falling_damage_runs[] = {{0, 4, 38}};
static struct tag_swap_run const struct_game_globals_first_person_interface_runs[] = {{0, 4, 12}, {48, 2, 2}, {52, 4, 4}, {68, 2, 2}, {72, 4, 30}};
static struct tag_swap_run const struct_game_globals_grenade_runs[] = {{0, 2, 2}, {4, 4, 16}};
static struct tag_swap_run const struct_game_globals_interface_tag_references_runs[] = {{0, 4, 76}};
static struct tag_swap_run const struct_game_globals_multiplayer_information_runs[] = {{0, 4, 26}};
static struct tag_swap_run const struct_game_globals_player_control_runs[] = {{0, 4, 27}, {108, 2, 2}, {112, 4, 4}};
static struct tag_swap_run const struct_game_globals_player_information_runs[] = {{0, 4, 61}};
static struct tag_swap_run const struct_game_globals_rasterizer_data_runs[] = {{0, 4, 92}, {368, 2, 2}, {372, 4, 14}};
static struct tag_swap_run const struct_game_globals_vehicle_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_gbxmodel_geometry_part_runs[] = {{0, 4, 1}, {4, 2, 1}, {8, 2, 2}, {12, 4, 14}, {68, 2, 2}, {72, 4, 3}, {84, 2, 2}, {88, 4, 4}, {130, 2, 1}};
static struct tag_swap_run const struct_glow_definition_runs[] = {{32, 2, 4}, {40, 4, 8}, {72, 2, 2}, {76, 4, 1}, {80, 2, 2}, {84, 4, 3}, {96, 2, 2}, {100, 4, 3}, {112, 2, 2}, {116, 4, 3}, {128, 2, 2}, {132, 4, 6}, {156, 2, 2}, {160, 4, 4}, {176, 2, 2}, {180, 4, 40}};
static struct tag_swap_run const struct_grenade_count_panel_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 13}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 4}, {104, 2, 2}, {108, 4, 2}, {116, 2, 2}, {120, 4, 9}, {156, 2, 2}, {160, 4, 2}, {168, 2, 2}, {176, 4, 3}, {188, 2, 2}};
static struct tag_swap_run const struct_grenade_hud_interface_definition_runs[] = {{0, 2, 2}, {4, 4, 8}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 13}, {104, 2, 2}, {108, 4, 2}, {116, 2, 4}, {124, 4, 4}, {140, 2, 2}, {144, 4, 2}, {152, 2, 2}, {156, 4, 13}, {208, 2, 2}, {212, 4, 2}, {220, 2, 4}, {228, 4, 4}, {244, 2, 2}, {248, 4, 2}, {256, 2, 2}, {260, 4, 9}, {296, 2, 2}, {300, 4, 2}, {308, 2, 2}, {316, 4, 3}, {328, 2, 2}, {332, 4, 43}};
static struct tag_swap_run const struct_hs_global_runs[] = {{32, 2, 2}, {36, 4, 2}};
static struct tag_swap_run const struct_hs_reference_runs[] = {{24, 4, 4}};
static struct tag_swap_run const struct_hs_script_runs[] = {{32, 2, 2}, {36, 4, 1}};
static struct tag_swap_run const struct_hs_source_file_runs[] = {{32, 4, 5}};
static struct tag_swap_run const struct_hud_absolute_placement_definition_runs[] = {{0, 2, 2}, {4, 4, 8}};
static struct tag_swap_run const struct_hud_globals_definition_runs[] = {{0, 2, 2}, {4, 4, 8}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 43}, {224, 2, 2}, {228, 4, 2}, {236, 2, 2}, {240, 4, 8}, {272, 2, 2}, {276, 4, 2}, {284, 2, 2}, {288, 4, 111}, {732, 2, 4}, {740, 4, 11}, {784, 2, 4}, {792, 4, 12}, {840, 2, 2}, {844, 4, 9}, {880, 2, 2}, {884, 4, 2}, {892, 2, 2}, {896, 4, 4}, {912, 2, 2}, {916, 4, 2}, {924, 2, 2}, {928, 4, 14}, {984, 2, 4}, {992, 4, 28}};
static struct tag_swap_run const struct_hud_message_text_definition_runs[] = {{0, 4, 32}};
static struct tag_swap_run const struct_hud_number_definition_runs[] = {{0, 4, 4}, {22, 2, 1}, {24, 4, 19}};
static struct tag_swap_run const struct_hud_screen_effect_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 14}, {64, 2, 2}, {68, 4, 10}, {108, 2, 2}, {112, 4, 7}, {140, 2, 2}, {144, 4, 10}};
static struct tag_swap_run const struct_hud_sound_definition_runs[] = {{0, 4, 6}};
static struct tag_swap_run const struct_hud_state_message_definition_runs[] = {{32, 2, 2}, {40, 4, 6}};
static struct tag_swap_run const struct_hud_state_message_element_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_hud_waypoint_arrow_runs[] = {{32, 4, 5}, {52, 2, 4}, {60, 4, 11}};
static struct tag_swap_run const struct_icon_hud_element_definition_runs[] = {{0, 2, 4}, {8, 4, 1}, {14, 2, 1}};
static struct tag_swap_run const struct_item_collection_definition_runs[] = {{0, 4, 3}, {12, 2, 2}, {16, 4, 19}};
static struct tag_swap_run const struct_item_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 16}, {384, 2, 2}, {388, 4, 1}, {392, 2, 2}, {396, 4, 4}, {412, 2, 4}, {420, 4, 89}};
static struct tag_swap_run const struct_item_permutation_definition_runs[] = {{0, 4, 21}};
static struct tag_swap_run const struct_leaf_map_runs[] = {{0, 4, 7}};
static struct tag_swap_run const struct_leaf_portal_runs[] = {{0, 4, 6}};
static struct tag_swap_run const struct_lens_flare_definition_runs[] = {{0, 4, 5}, {20, 2, 2}, {24, 4, 6}, {48, 2, 2}, {128, 2, 2}, {132, 4, 1}, {160, 4, 2}, {196, 4, 3}};
static struct tag_swap_run const struct_lens_flare_reflection_runs[] = {{0, 2, 4}, {28, 4, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 2}, {60, 2, 2}, {64, 4, 12}, {112, 2, 2}, {116, 4, 2}};
static struct tag_swap_run const struct_light_definition_runs[] = {{0, 4, 11}, {52, 4, 9}, {100, 4, 4}, {116, 2, 2}, {120, 4, 5}, {140, 2, 2}, {144, 4, 1}, {148, 2, 2}, {152, 4, 1}, {156, 2, 2}, {160, 4, 1}, {172, 4, 4}, {244, 4, 1}, {248, 2, 2}};
static struct tag_swap_run const struct_light_fixture_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}, {40, 2, 2}, {44, 4, 7}};
static struct tag_swap_run const struct_light_volume_definition_runs[] = {{32, 2, 2}, {36, 4, 8}, {68, 2, 2}, {72, 4, 9}, {108, 2, 2}, {112, 4, 18}, {184, 2, 2}, {188, 4, 36}};
static struct tag_swap_run const struct_light_volume_frame_runs[] = {{0, 4, 44}};
static struct tag_swap_run const struct_lightning_definition_runs[] = {{0, 2, 2}, {20, 4, 2}, {44, 2, 4}, {52, 4, 4}, {152, 4, 6}};
static struct tag_swap_run const struct_lightning_marker_definition_runs[] = {{32, 2, 4}, {116, 4, 9}};
static struct tag_swap_run const struct_lightning_shader_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 8}, {76, 4, 4}, {92, 2, 4}, {100, 4, 3}, {112, 2, 2}, {116, 4, 3}, {128, 2, 2}, {132, 4, 7}};
static struct tag_swap_run const struct_looping_sound_definition_runs[] = {{0, 4, 21}};
static struct tag_swap_run const struct_looping_sound_detail_runs[] = {{0, 4, 26}};
static struct tag_swap_run const struct_looping_sound_track_runs[] = {{0, 4, 40}};
static struct tag_swap_run const struct_machine_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 22}, {408, 2, 4}, {416, 4, 60}, {656, 2, 2}, {660, 4, 21}, {744, 2, 2}, {748, 4, 14}};
static struct tag_swap_run const struct_machine_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}, {40, 2, 2}, {44, 4, 1}};
static struct tag_swap_run const struct_map_leaf_runs[] = {{0, 4, 6}};
static struct tag_swap_run const struct_map_leaf_face_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_mass_point_definition_runs[] = {{32, 2, 2}, {36, 4, 14}, {92, 2, 2}, {96, 4, 3}};
static struct tag_swap_run const struct_material_definition_runs[] = {{0, 4, 221}};
static struct tag_swap_run const struct_material_effect_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_material_effect_material_runs[] = {{0, 4, 8}};
static struct tag_swap_run const struct_material_effects_definition_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_meter_hud_element_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 13}, {70, 2, 1}, {74, 2, 1}, {76, 4, 7}};
static struct tag_swap_run const struct_metered_panel_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 13}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 4}, {104, 2, 2}, {108, 4, 2}, {116, 2, 2}, {120, 4, 13}, {174, 2, 1}, {178, 2, 1}, {180, 4, 15}};
static struct tag_swap_run const struct_model_runs[] = {{0, 4, 7}, {28, 2, 6}, {40, 4, 48}};
static struct tag_swap_run const struct_model_geometry_runs[] = {{36, 4, 3}};
static struct tag_swap_run const struct_model_geometry_part_runs[] = {{0, 4, 1}, {4, 2, 1}, {8, 2, 2}, {12, 4, 14}, {68, 2, 2}, {72, 4, 3}, {84, 2, 2}, {88, 4, 4}};
static struct tag_swap_run const struct_model_marker_runs[] = {{32, 4, 8}};
static struct tag_swap_run const struct_model_marker_instance_runs[] = {{4, 4, 7}};
static struct tag_swap_run const struct_model_node_runs[] = {{32, 2, 4}, {40, 4, 29}};
static struct tag_swap_run const struct_model_region_runs[] = {{32, 4, 11}};
static struct tag_swap_run const struct_model_region_permutation_runs[] = {{32, 4, 1}, {36, 2, 2}, {40, 4, 6}, {64, 2, 6}, {76, 4, 3}};
static struct tag_swap_run const struct_model_region_permutation_marker_runs[] = {{32, 2, 2}, {36, 4, 7}};
static struct tag_swap_run const struct_model_shader_reference_runs[] = {{0, 4, 4}, {16, 2, 2}, {20, 4, 3}};
static struct tag_swap_run const struct_model_triangle_element_runs[] = {{0, 2, 3}};
static struct tag_swap_run const struct_model_vertex_compressed_element_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_model_vertex_uncompressed_element_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_motion_sensor_panel_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 13}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 4}, {104, 2, 2}, {108, 4, 2}, {116, 2, 2}, {120, 4, 13}, {172, 2, 2}, {176, 4, 2}, {184, 2, 4}, {192, 4, 12}};
static struct tag_swap_run const struct_move_position_definition_runs[] = {{0, 4, 7}, {28, 2, 1}, {32, 4, 2}, {40, 2, 2}, {44, 4, 9}};
static struct tag_swap_run const struct_multiplayer_scenario_description_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_multiplayer_scenario_description_entry_runs[] = {{0, 4, 8}, {64, 4, 1}};
static struct tag_swap_run const struct_multitexture_overlay_hud_element_definition_runs[] = {{0, 2, 4}, {8, 4, 8}, {40, 2, 6}, {52, 4, 24}, {148, 2, 4}, {156, 4, 81}};
static struct tag_swap_run const struct_multitexture_overlay_hud_element_effector_definition_runs[] = {{0, 4, 16}, {64, 2, 4}, {72, 4, 26}, {176, 2, 2}, {180, 4, 10}};
static struct tag_swap_run const struct_object_attachment_definition_runs[] = {{0, 4, 4}, {48, 2, 4}, {56, 4, 4}};
static struct tag_swap_run const struct_object_change_color_definition_runs[] = {{0, 2, 2}, {4, 4, 10}};
static struct tag_swap_run const struct_object_change_color_permutation_runs[] = {{0, 4, 7}};
static struct tag_swap_run const struct_object_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 15}};
static struct tag_swap_run const struct_object_definition_widget_runs[] = {{0, 4, 8}};
static struct tag_swap_run const struct_object_function_definition_runs[] = {{0, 4, 2}, {8, 2, 4}, {16, 4, 3}, {28, 2, 6}, {40, 4, 3}, {52, 2, 2}, {56, 4, 68}};
static struct tag_swap_run const struct_optional_meter_hud_element_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 13}, {70, 2, 1}, {74, 2, 1}, {76, 4, 7}};
static struct tag_swap_run const struct_optional_static_hud_element_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 13}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 4}};
static struct tag_swap_run const struct_particle_definition_runs[] = {{0, 4, 38}, {152, 2, 4}, {160, 4, 3}, {172, 2, 4}, {180, 4, 7}, {208, 2, 8}, {252, 4, 4}, {268, 2, 4}, {276, 4, 3}, {288, 2, 2}, {292, 4, 3}, {304, 2, 2}, {308, 4, 7}};
static struct tag_swap_run const struct_particle_system_definition_runs[] = {{56, 4, 4}, {72, 2, 2}, {76, 4, 7}};
static struct tag_swap_run const struct_particle_system_physics_constant_runs[] = {{0, 4, 1}};
static struct tag_swap_run const struct_particle_system_type_runs[] = {{32, 4, 1}, {36, 2, 4}, {44, 4, 1}, {84, 2, 2}, {88, 4, 10}};
static struct tag_swap_run const struct_particle_system_type_particle_state_runs[] = {{32, 4, 8}, {64, 2, 2}, {68, 4, 20}, {184, 2, 2}, {188, 4, 7}, {216, 2, 8}, {260, 4, 4}, {276, 2, 4}, {284, 4, 3}, {296, 2, 2}, {300, 4, 3}, {312, 2, 2}, {316, 4, 7}};
static struct tag_swap_run const struct_particle_system_type_state_runs[] = {{32, 4, 36}, {176, 2, 2}, {180, 4, 3}};
static struct tag_swap_run const struct_pathfinding_sphere_runs[] = {{0, 2, 2}, {4, 4, 7}};
static struct tag_swap_run const struct_physics_definition_runs[] = {{0, 4, 32}};
static struct tag_swap_run const struct_platoon_definition_runs[] = {{32, 4, 4}, {48, 2, 2}, {52, 4, 2}, {60, 2, 2}, {64, 4, 1}};
static struct tag_swap_run const struct_platoon_rule_runs[] = {{0, 2, 2}, {4, 4, 1}};
static struct tag_swap_run const struct_player_starting_location_runs[] = {{0, 4, 4}, {16, 2, 6}};
static struct tag_swap_run const struct_point_physics_definition_runs[] = {{0, 4, 16}};
static struct tag_swap_run const struct_powered_mass_point_definition_runs[] = {{32, 4, 24}};
static struct tag_swap_run const struct_powered_seat_definition_runs[] = {{0, 4, 17}};
static struct tag_swap_run const struct_predicted_resource_runs[] = {{0, 2, 2}, {4, 4, 1}};
static struct tag_swap_run const struct_projectile_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 16}, {384, 2, 6}, {396, 4, 25}, {496, 2, 2}, {500, 4, 22}};
static struct tag_swap_run const struct_projectile_material_response_definition_runs[] = {{0, 2, 2}, {4, 4, 4}, {36, 2, 2}, {40, 4, 9}, {92, 2, 2}, {96, 4, 6}, {144, 4, 4}};
static struct tag_swap_run const struct_recorded_animation_definition_runs[] = {{36, 2, 2}, {40, 4, 6}};
static struct tag_swap_run const struct_render_lighting_runs[] = {{0, 4, 3}, {12, 2, 2}, {16, 4, 12}, {64, 2, 2}, {68, 4, 12}};
static struct tag_swap_run const struct_scenario_runs[] = {{0, 4, 15}, {60, 2, 2}, {64, 4, 348}};
static struct tag_swap_run const struct_scenario_bsp_switch_trigger_volume_runs[] = {{0, 2, 4}};
static struct tag_swap_run const struct_scenario_conversation_line_runs[] = {{0, 2, 4}, {8, 4, 2}, {28, 4, 24}};
static struct tag_swap_run const struct_scenario_conversation_participant_runs[] = {{0, 2, 6}, {24, 2, 6}, {68, 4, 1}};
static struct tag_swap_run const struct_scenario_cutscene_camera_point_runs[] = {{0, 4, 1}, {36, 4, 17}};
static struct tag_swap_run const struct_scenario_cutscene_flag_runs[] = {{0, 4, 1}, {36, 4, 5}};
static struct tag_swap_run const struct_scenario_cutscene_title_runs[] = {{0, 4, 1}, {36, 4, 1}, {40, 2, 8}, {56, 4, 6}};
static struct tag_swap_run const struct_scenario_decal_palette_entry_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_scenario_detail_object_collection_palette_entry_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_scenario_device_group_runs[] = {{32, 4, 5}};
static struct tag_swap_run const struct_scenario_netgame_equipment_runs[] = {{0, 4, 1}, {4, 2, 6}, {16, 4, 32}};
static struct tag_swap_run const struct_scenario_netgame_flag_runs[] = {{0, 4, 4}, {16, 2, 2}, {20, 4, 32}};
static struct tag_swap_run const struct_scenario_object_name_runs[] = {{32, 2, 2}};
static struct tag_swap_run const struct_scenario_object_palette_entry_runs[] = {{0, 4, 12}};
static struct tag_swap_run const struct_scenario_starting_equipment_runs[] = {{0, 4, 1}, {4, 2, 4}, {12, 4, 48}};
static struct tag_swap_run const struct_scenario_starting_profile_runs[] = {{32, 4, 6}, {56, 2, 2}, {60, 4, 4}, {76, 2, 2}};
static struct tag_swap_run const struct_scenario_structure_bsp_reference_runs[] = {{0, 4, 3}, {16, 4, 4}};
static struct tag_swap_run const struct_scenario_tag_reference_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_scenario_trigger_volume_runs[] = {{0, 2, 2}, {48, 4, 12}};
static struct tag_swap_run const struct_scenario_weapon_datum_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 5}, {64, 4, 2}, {72, 2, 4}, {80, 4, 3}};
static struct tag_swap_run const struct_scenery_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 5}, {64, 4, 2}};
static struct tag_swap_run const struct_shader_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}};
static struct tag_swap_run const struct_shader_effect_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 8}, {76, 4, 4}, {92, 2, 4}, {100, 4, 3}, {112, 2, 2}, {116, 4, 3}, {128, 2, 2}, {132, 4, 7}};
static struct tag_swap_run const struct_shader_environment_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {44, 4, 5}, {108, 2, 2}, {136, 4, 4}, {176, 2, 2}, {180, 4, 10}, {244, 2, 2}, {248, 4, 8}, {292, 4, 7}, {336, 2, 2}, {340, 4, 2}, {348, 2, 2}, {352, 4, 2}, {384, 2, 2}, {412, 4, 6}, {436, 2, 2}, {440, 4, 2}, {472, 4, 6}, {496, 2, 2}, {500, 4, 2}, {532, 4, 6}, {556, 2, 2}, {560, 4, 2}, {592, 4, 5}, {636, 2, 2}, {720, 2, 2}, {804, 4, 4}};
static struct tag_swap_run const struct_shader_layer_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_shader_model_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {56, 4, 1}, {76, 2, 1}, {108, 2, 4}, {116, 4, 7}, {156, 4, 6}, {188, 4, 4}, {212, 2, 2}, {216, 4, 6}, {252, 2, 2}, {256, 4, 3}, {268, 2, 2}, {272, 4, 3}, {284, 2, 2}, {288, 4, 5}, {316, 4, 14}};
static struct tag_swap_run const struct_shader_texture_animation_runs[] = {{0, 2, 2}, {4, 4, 3}, {16, 2, 2}, {20, 4, 3}, {32, 2, 2}, {36, 4, 5}};
static struct tag_swap_run const struct_shader_transparent_runs[] = {{2, 2, 5}, {12, 4, 11}};
static struct tag_swap_run const struct_shader_transparent_chicago_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}, {42, 2, 5}, {52, 4, 12}};
static struct tag_swap_run const struct_shader_transparent_chicago_extended_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}, {42, 2, 5}, {52, 4, 15}};
static struct tag_swap_run const struct_shader_transparent_chicago_map_runs[] = {{0, 2, 1}, {44, 2, 2}, {84, 4, 10}, {164, 2, 2}, {168, 4, 3}, {180, 2, 2}, {184, 4, 3}, {196, 2, 2}, {200, 4, 5}};
static struct tag_swap_run const struct_shader_transparent_generic_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}, {42, 2, 5}, {52, 4, 14}};
static struct tag_swap_run const struct_shader_transparent_generic_map_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 2, 2}, {48, 4, 3}, {60, 2, 2}, {64, 4, 3}, {76, 2, 2}, {80, 4, 5}};
static struct tag_swap_run const struct_shader_transparent_generic_stage_runs[] = {{0, 2, 4}, {8, 4, 13}, {60, 2, 26}};
static struct tag_swap_run const struct_shader_transparent_glass_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {84, 4, 8}, {136, 2, 2}, {140, 4, 17}, {336, 2, 2}, {340, 4, 10}};
static struct tag_swap_run const struct_shader_transparent_meter_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {76, 4, 4}};
static struct tag_swap_run const struct_shader_transparent_plasma_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}, {44, 2, 2}, {48, 4, 1}, {52, 2, 2}, {56, 4, 2}, {96, 4, 8}, {128, 2, 1}, {192, 4, 9}, {264, 4, 9}};
static struct tag_swap_run const struct_shader_transparent_water_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {76, 4, 4}, {108, 4, 8}, {156, 4, 4}, {188, 4, 7}, {216, 2, 2}, {220, 4, 2}, {292, 4, 3}};
static struct tag_swap_run const struct_sky_animation_runs[] = {{0, 2, 2}, {4, 4, 1}};
static struct tag_swap_run const struct_sky_definition_runs[] = {{0, 4, 8}, {152, 4, 14}};
static struct tag_swap_run const struct_sky_light_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_sky_shader_function_runs[] = {{0, 4, 1}};
static struct tag_swap_run const struct_sound_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 10}, {60, 4, 3}, {84, 4, 3}, {108, 2, 2}, {112, 4, 4}, {128, 2, 2}, {132, 4, 8}};
static struct tag_swap_run const struct_sound_environment_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 12}};
static struct tag_swap_run const struct_sound_permutation_runs[] = {{32, 4, 2}, {40, 2, 2}, {44, 4, 20}};
static struct tag_swap_run const struct_sound_pitch_range_runs[] = {{32, 4, 3}, {44, 2, 2}, {48, 4, 2}, {56, 2, 2}, {60, 4, 3}};
static struct tag_swap_run const struct_sound_scenery_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}};
static struct tag_swap_run const struct_squad_definition_runs[] = {{32, 2, 4}, {40, 4, 1}, {44, 2, 2}, {48, 4, 7}, {76, 2, 2}, {80, 4, 11}, {124, 2, 8}, {140, 4, 23}};
static struct tag_swap_run const struct_static_hud_element_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 13}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 4}};
static struct tag_swap_run const struct_string_list_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_string_list_entry_runs[] = {{0, 4, 5}};
static struct tag_swap_run const struct_structure_background_sound_palette_entry_runs[] = {{32, 4, 4}};
static struct tag_swap_run const struct_structure_breakable_surface_runs[] = {{0, 4, 12}};
static struct tag_swap_run const struct_structure_bsp_runs[] = {{0, 4, 14}, {56, 2, 2}, {60, 4, 12}, {108, 2, 2}, {112, 4, 134}};
static struct tag_swap_run const struct_structure_cluster_portal_runs[] = {{0, 2, 2}, {4, 4, 15}};
static struct tag_swap_run const struct_structure_cluster_schema_runs[] = {{0, 2, 8}, {16, 4, 12}, {64, 2, 2}, {68, 4, 9}};
static struct tag_swap_run const struct_structure_collision_material_runs[] = {{0, 4, 4}, {16, 2, 2}};
static struct tag_swap_run const struct_structure_detail_object_data_runs[] = {{0, 4, 12}, {52, 4, 3}};
static struct tag_swap_run const struct_structure_fog_palette_entry_runs[] = {{32, 4, 4}};
static struct tag_swap_run const struct_structure_fog_plane_schema_runs[] = {{0, 2, 2}, {4, 4, 7}};
static struct tag_swap_run const struct_structure_fog_region_runs[] = {{36, 2, 2}};
static struct tag_swap_run const struct_structure_leaf_runs[] = {{6, 2, 3}, {12, 4, 1}};
static struct tag_swap_run const struct_structure_lens_flare_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_structure_lens_flare_marker_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_structure_lightmap_runs[] = {{0, 2, 2}, {4, 4, 7}};
static struct tag_swap_run const struct_structure_material_runs[] = {{0, 4, 4}, {16, 2, 2}, {20, 4, 8}, {52, 2, 2}, {56, 4, 12}, {104, 2, 2}, {108, 4, 16}, {172, 2, 4}, {180, 4, 4}, {196, 2, 2}, {200, 4, 14}};
static struct tag_swap_run const struct_structure_mirror_runs[] = {{0, 4, 16}};
static struct tag_swap_run const struct_structure_portal_index_runs[] = {{0, 2, 1}};
static struct tag_swap_run const struct_structure_runtime_decal_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_structure_sound_environment_palette_entry_runs[] = {{32, 4, 4}};
static struct tag_swap_run const struct_structure_subcluster_runs[] = {{0, 4, 9}};
static struct tag_swap_run const struct_structure_surface_runs[] = {{0, 2, 3}};
static struct tag_swap_run const struct_structure_surface_reference_runs[] = {{0, 4, 2}};
static struct tag_swap_run const struct_structure_weather_palette_entry_runs[] = {{32, 4, 4}, {48, 2, 2}, {84, 4, 19}, {160, 2, 2}, {196, 4, 11}};
static struct tag_swap_run const struct_structure_weather_polyhedron_runs[] = {{0, 4, 8}};
static struct tag_swap_run const struct_tag_reference_element_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_ui_widget_child_reference_runs[] = {{0, 4, 4}, {48, 4, 1}, {52, 2, 3}};
static struct tag_swap_run const struct_ui_widget_collection_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_ui_widget_collection_widget_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_ui_widget_conditional_reference_runs[] = {{0, 4, 4}, {48, 4, 1}, {52, 2, 1}};
static struct tag_swap_run const struct_ui_widget_definition_runs[] = {{0, 2, 2}, {36, 2, 4}, {44, 4, 16}, {236, 4, 12}, {284, 2, 2}, {302, 2, 3}, {336, 4, 9}, {372, 2, 8}, {420, 4, 4}, {724, 4, 3}, {992, 4, 3}};
static struct tag_swap_run const struct_ui_widget_event_handler_reference_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 8}};
static struct tag_swap_run const struct_ui_widget_game_data_input_reference_runs[] = {{0, 2, 1}};
static struct tag_swap_run const struct_ui_widget_search_and_replace_reference_runs[] = {{32, 2, 1}};
static struct tag_swap_run const struct_unit_camera_runs[] = {{64, 4, 6}};
static struct tag_swap_run const struct_unit_camera_track_runs[] = {{0, 4, 7}};
static struct tag_swap_run const struct_unit_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 16}, {384, 2, 2}, {388, 4, 5}, {408, 2, 4}, {416, 4, 2}, {488, 4, 29}, {604, 2, 2}, {608, 4, 14}, {664, 2, 2}, {668, 4, 10}, {708, 2, 4}, {716, 4, 9}};
static struct tag_swap_run const struct_unit_dialogue_variant_definition_runs[] = {{0, 2, 2}, {4, 4, 5}};
static struct tag_swap_run const struct_unit_hud_interface_definition_runs[] = {{0, 2, 2}, {4, 4, 8}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 13}, {104, 2, 2}, {108, 4, 2}, {116, 2, 4}, {124, 4, 4}, {140, 2, 2}, {144, 4, 2}, {152, 2, 2}, {156, 4, 13}, {208, 2, 2}, {212, 4, 2}, {220, 2, 4}, {228, 4, 4}, {244, 2, 2}, {248, 4, 2}, {256, 2, 2}, {260, 4, 13}, {314, 2, 1}, {318, 2, 1}, {320, 4, 15}, {380, 2, 2}, {384, 4, 2}, {392, 2, 2}, {396, 4, 13}, {448, 2, 2}, {452, 4, 2}, {460, 2, 4}, {468, 4, 4}, {484, 2, 2}, {488, 4, 2}, {496, 2, 2}, {500, 4, 13}, {554, 2, 1}, {558, 2, 1}, {560, 4, 15}, {620, 2, 2}, {624, 4, 2}, {632, 2, 2}, {636, 4, 13}, {688, 2, 2}, {692, 4, 2}, {700, 2, 4}, {708, 4, 4}, {724, 2, 2}, {728, 4, 2}, {736, 2, 2}, {740, 4, 13}, {792, 2, 2}, {796, 4, 2}, {804, 2, 4}, {812, 4, 12}, {860, 2, 2}, {864, 4, 2}, {872, 2, 2}, {876, 4, 5}, {896, 2, 2}, {900, 4, 122}};
static struct tag_swap_run const struct_unit_hud_reference_runs[] = {{0, 4, 12}};
static struct tag_swap_run const struct_unit_initial_weapon_runs[] = {{0, 4, 9}};
static struct tag_swap_run const struct_unit_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 5}, {64, 4, 4}};
static struct tag_swap_run const struct_unit_seat_runs[] = {{0, 4, 1}, {68, 4, 16}, {196, 4, 10}, {236, 2, 2}, {240, 4, 11}};
static struct tag_swap_run const struct_vehicle_animation_runs[] = {{0, 4, 2}, {8, 2, 2}, {12, 4, 2}, {20, 2, 2}, {24, 4, 23}};
static struct tag_swap_run const struct_vehicle_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 16}, {384, 2, 2}, {388, 4, 5}, {408, 2, 4}, {416, 4, 2}, {488, 4, 29}, {604, 2, 2}, {608, 4, 14}, {664, 2, 2}, {668, 4, 10}, {708, 2, 4}, {716, 4, 10}, {756, 2, 2}, {760, 4, 9}, {796, 2, 4}, {816, 4, 2}, {832, 4, 2}, {868, 4, 1}, {944, 4, 16}};
static struct tag_swap_run const struct_vehicle_suspension_runs[] = {{0, 2, 2}, {4, 4, 2}};
static struct tag_swap_run const struct_vibrate_frequency_definition_runs[] = {{0, 4, 2}, {8, 2, 2}, {12, 4, 2}};
static struct tag_swap_run const struct_virtual_keyboard_definition_runs[] = {{0, 4, 15}};
static struct tag_swap_run const struct_virtual_keyboard_key_runs[] = {{0, 2, 8}, {16, 4, 16}};
static struct tag_swap_run const struct_water_ripple_runs[] = {{4, 4, 1}, {40, 4, 4}, {56, 2, 2}};
static struct tag_swap_run const struct_weapon_ammunition_object_runs[] = {{0, 2, 6}, {12, 4, 4}};
static struct tag_swap_run const struct_weapon_definition_runs[] = {{0, 2, 2}, {4, 4, 65}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 16}, {384, 2, 2}, {388, 4, 1}, {392, 2, 2}, {396, 4, 4}, {412, 2, 4}, {420, 4, 90}, {812, 2, 6}, {824, 4, 40}, {984, 2, 2}, {988, 4, 8}, {1020, 2, 2}, {1024, 4, 56}, {1248, 2, 2}, {1252, 4, 9}};
static struct tag_swap_run const struct_weapon_firing_effect_runs[] = {{0, 2, 2}, {4, 4, 32}};
static struct tag_swap_run const struct_weapon_hud_crosshair_item_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 9}, {52, 2, 2}, {56, 4, 2}, {64, 2, 4}, {72, 4, 9}};
static struct tag_swap_run const struct_weapon_hud_crosshairs_element_runs[] = {{0, 2, 4}, {8, 4, 24}};
static struct tag_swap_run const struct_weapon_hud_element_header_runs[] = {{0, 2, 4}, {8, 4, 7}};
static struct tag_swap_run const struct_weapon_hud_interface_definition_runs[] = {{0, 4, 4}, {16, 2, 6}, {28, 4, 8}, {60, 2, 2}, {64, 4, 63}, {316, 2, 4}, {324, 4, 1}, {330, 2, 1}, {332, 4, 12}};
static struct tag_swap_run const struct_weapon_hud_meter_element_runs[] = {{0, 2, 4}, {8, 4, 7}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 13}, {106, 2, 1}, {110, 2, 1}, {112, 4, 17}};
static struct tag_swap_run const struct_weapon_hud_number_element_runs[] = {{0, 2, 4}, {8, 4, 7}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 9}, {88, 2, 2}, {92, 4, 2}, {100, 2, 2}, {108, 4, 3}, {120, 2, 2}, {124, 4, 9}};
static struct tag_swap_run const struct_weapon_hud_overlay_definition_runs[] = {{0, 4, 7}};
static struct tag_swap_run const struct_weapon_hud_overlay_item_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 9}, {52, 2, 2}, {56, 4, 2}, {64, 2, 6}, {76, 4, 15}};
static struct tag_swap_run const struct_weapon_hud_overlays_element_runs[] = {{0, 2, 4}, {8, 4, 24}};
static struct tag_swap_run const struct_weapon_hud_static_element_runs[] = {{0, 2, 4}, {8, 4, 7}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 13}, {104, 2, 2}, {108, 4, 2}, {116, 2, 4}, {124, 4, 14}};
static struct tag_swap_run const struct_weapon_magazine_definition_runs[] = {{0, 4, 1}, {4, 2, 8}, {20, 4, 1}, {24, 2, 2}, {28, 4, 3}, {40, 2, 8}, {56, 4, 14}};
static struct tag_swap_run const struct_weapon_trigger_definition_runs[] = {{0, 4, 8}, {32, 2, 8}, {48, 4, 8}, {80, 2, 2}, {84, 4, 6}, {108, 2, 2}, {112, 4, 41}};
static struct tag_swap_run const struct_weather_particle_system_definition_runs[] = {{0, 4, 12}};
static struct tag_swap_run const struct_weather_particle_type_definition_runs[] = {{32, 4, 97}, {420, 2, 4}, {428, 4, 7}, {456, 2, 8}, {500, 4, 4}, {516, 2, 4}, {524, 4, 3}, {536, 2, 2}, {540, 4, 3}, {552, 2, 2}, {556, 4, 7}};
static struct tag_swap_run const struct_wind_definition_runs[] = {{0, 4, 16}};
static struct tag_swap_run const word_runs[] = {{0, 2, 1}};

struct tag_swap_type const tag_swap_types[] =
{
	{"byte", 1, 0, byte_runs},
	{"byte_rectangle3d", 6, 0, byte_rectangle3d_runs},
	{"long", 4, 1, long_runs},
	{"real", 4, 1, real_runs},
	{"real_matrix3x3", 36, 1, real_matrix3x3_runs},
	{"real_plane3d", 16, 1, real_plane3d_runs},
	{"real_point2d", 8, 1, real_point2d_runs},
	{"real_point3d", 12, 1, real_point3d_runs},
	{"real_vector4d", 16, 1, real_vector4d_runs},
	{"struct _biped_definition", 516, 5, struct__biped_definition_runs},
	{"struct _control_definition", 148, 3, struct__control_definition_runs},
	{"struct _device_definition", 276, 3, struct__device_definition_runs},
	{"struct _equipment_definition", 24, 2, struct__equipment_definition_runs},
	{"struct _item_definition", 396, 7, struct__item_definition_runs},
	{"struct _machine_definition", 148, 4, struct__machine_definition_runs},
	{"struct _object_definition", 380, 6, struct__object_definition_runs},
	{"struct _projectile_definition", 208, 5, struct__projectile_definition_runs},
	{"struct _unit_definition", 372, 12, struct__unit_definition_runs},
	{"struct _weapon_definition", 512, 9, struct__weapon_definition_runs},
	{"struct actor_definition", 1272, 13, struct_actor_definition_runs},
	{"struct actor_starting_location", 28, 3, struct_actor_starting_location_runs},
	{"struct actor_variant_change_colors", 32, 1, struct_actor_variant_change_colors_runs},
	{"struct actor_variant_definition", 568, 13, struct_actor_variant_definition_runs},
	{"struct ai_animation_reference_definition", 60, 1, struct_ai_animation_reference_definition_runs},
	{"struct ai_command_definition", 32, 4, struct_ai_command_definition_runs},
	{"struct ai_command_list_definition", 96, 3, struct_ai_command_list_definition_runs},
	{"struct ai_command_point_definition", 20, 1, struct_ai_command_point_definition_runs},
	{"struct ai_conversation", 116, 3, struct_ai_conversation_runs},
	{"struct ai_recording_reference_definition", 40, 1, struct_ai_recording_reference_definition_runs},
	{"struct ai_script_reference_definition", 40, 1, struct_ai_script_reference_definition_runs},
	{"struct animation", 180, 7, struct_animation_runs},
	{"struct animation_graph", 128, 3, struct_animation_graph_runs},
	{"struct animation_graph_animation_index", 2, 1, struct_animation_graph_animation_index_runs},
	{"struct animation_graph_device_animations", 96, 1, struct_animation_graph_device_animations_runs},
	{"struct animation_graph_ik_point", 64, 0, struct_animation_graph_ik_point_runs},
	{"struct animation_graph_node", 64, 2, struct_animation_graph_node_runs},
	{"struct animation_graph_object_overlay", 20, 2, struct_animation_graph_object_overlay_runs},
	{"struct animation_graph_sound_reference", 20, 1, struct_animation_graph_sound_reference_runs},
	{"struct animation_graph_unit_seat", 100, 5, struct_animation_graph_unit_seat_runs},
	{"struct animation_graph_weapon_animations", 28, 1, struct_animation_graph_weapon_animations_runs},
	{"struct animation_graph_weapon_class", 188, 5, struct_animation_graph_weapon_class_runs},
	{"struct animation_graph_weapon_type", 60, 1, struct_animation_graph_weapon_type_runs},
	{"struct antenna_definition", 208, 1, struct_antenna_definition_runs},
	{"struct antenna_vertex_definition", 128, 3, struct_antenna_vertex_definition_runs},
	{"struct auxilary_meter_definition", 324, 17, struct_auxilary_meter_definition_runs},
	{"struct auxilary_overlay_definition", 132, 10, struct_auxilary_overlay_definition_runs},
	{"struct biped_contact_point", 64, 0, struct_biped_contact_point_runs},
	{"struct biped_definition", 1268, 21, struct_biped_definition_runs},
	{"struct bitmap_data", 48, 3, struct_bitmap_data_runs},
	{"struct bitmap_group", 108, 6, struct_bitmap_group_runs},
	{"struct bitmap_group_sequence", 64, 2, struct_bitmap_group_sequence_runs},
	{"struct bitmap_group_sprite", 32, 2, struct_bitmap_group_sprite_runs},
	{"struct breakable_surface_particle_effect", 128, 1, struct_breakable_surface_particle_effect_runs},
	{"struct bsp2d_node", 20, 1, struct_bsp2d_node_runs},
	{"struct bsp2d_reference", 8, 1, struct_bsp2d_reference_runs},
	{"struct bsp3d_node", 12, 1, struct_bsp3d_node_runs},
	{"struct camera_track_control_point", 60, 1, struct_camera_track_control_point_runs},
	{"struct camera_track_definition", 48, 1, struct_camera_track_definition_runs},
	{"struct collision_bsp", 96, 1, struct_collision_bsp_runs},
	{"struct collision_edge", 24, 1, struct_collision_edge_runs},
	{"struct collision_leaf", 8, 2, struct_collision_leaf_runs},
	{"struct collision_model", 664, 7, struct_collision_model_runs},
	{"struct collision_node", 64, 4, struct_collision_node_runs},
	{"struct collision_surface", 12, 2, struct_collision_surface_runs},
	{"struct collision_vertex", 16, 1, struct_collision_vertex_runs},
	{"struct color_table_color", 48, 1, struct_color_table_color_runs},
	{"struct color_table_definition", 12, 1, struct_color_table_definition_runs},
	{"struct continuous_damage_effect_definition", 512, 5, struct_continuous_damage_effect_definition_runs},
	{"struct contrail_definition", 324, 11, struct_contrail_definition_runs},
	{"struct contrail_point_state", 104, 1, struct_contrail_point_state_runs},
	{"struct control_definition", 804, 11, struct_control_definition_runs},
	{"struct control_placement", 64, 7, struct_control_placement_runs},
	{"struct damage_definition", 220, 2, struct_damage_definition_runs},
	{"struct damage_effect_definition", 672, 17, struct_damage_effect_definition_runs},
	{"struct damage_permutation", 32, 0, struct_damage_permutation_runs},
	{"struct damage_region", 84, 1, struct_damage_region_runs},
	{"struct damage_resistance", 600, 7, struct_damage_resistance_runs},
	{"struct damage_resistance_material", 72, 3, struct_damage_resistance_material_runs},
	{"struct decal_definition", 268, 10, struct_decal_definition_runs},
	{"struct detail_object", 6, 1, struct_detail_object_runs},
	{"struct detail_object_cell_definition", 32, 2, struct_detail_object_cell_definition_runs},
	{"struct detail_object_collection_definition", 128, 2, struct_detail_object_collection_definition_runs},
	{"struct detail_object_type_definition", 96, 1, struct_detail_object_type_definition_runs},
	{"struct device_definition", 656, 8, struct_device_definition_runs},
	{"struct dialogue_definition", 4112, 2, struct_dialogue_definition_runs},
	{"struct effect_definition", 64, 3, struct_effect_definition_runs},
	{"struct effect_event_definition", 68, 1, struct_effect_event_definition_runs},
	{"struct effect_location_definition", 32, 0, struct_effect_location_definition_runs},
	{"struct effect_part_definition", 104, 2, struct_effect_part_definition_runs},
	{"struct effect_particles_definition", 232, 4, struct_effect_particles_definition_runs},
	{"struct encounter_definition", 176, 5, struct_encounter_definition_runs},
	{"struct encounter_player_starting_location", 52, 2, struct_encounter_player_starting_location_runs},
	{"struct equipment_definition", 800, 14, struct_equipment_definition_runs},
	{"struct equipment_placement", 40, 4, struct_equipment_placement_runs},
	{"struct firing_position_definition", 24, 3, struct_firing_position_definition_runs},
	{"struct flag_attachment_point", 52, 2, struct_flag_attachment_point_runs},
	{"struct flag_definition", 96, 3, struct_flag_definition_runs},
	{"struct fog_definition_full", 396, 11, struct_fog_definition_full_runs},
	{"struct font_character", 20, 2, struct_font_character_runs},
	{"struct font_character_index", 2, 1, struct_font_character_index_runs},
	{"struct font_character_table", 12, 1, struct_font_character_table_runs},
	{"struct font_header", 156, 3, struct_font_header_runs},
	{"struct game_globals", 428, 1, struct_game_globals_runs},
	{"struct game_globals_difficulty_information", 644, 1, struct_game_globals_difficulty_information_runs},
	{"struct game_globals_falling_damage", 152, 1, struct_game_globals_falling_damage_runs},
	{"struct game_globals_first_person_interface", 192, 5, struct_game_globals_first_person_interface_runs},
	{"struct game_globals_grenade", 68, 2, struct_game_globals_grenade_runs},
	{"struct game_globals_interface_tag_references", 304, 1, struct_game_globals_interface_tag_references_runs},
	{"struct game_globals_multiplayer_information", 160, 1, struct_game_globals_multiplayer_information_runs},
	{"struct game_globals_player_control", 128, 3, struct_game_globals_player_control_runs},
	{"struct game_globals_player_information", 244, 1, struct_game_globals_player_information_runs},
	{"struct game_globals_rasterizer_data", 428, 3, struct_game_globals_rasterizer_data_runs},
	{"struct game_globals_vehicle", 16, 1, struct_game_globals_vehicle_runs},
	{"struct gbxmodel_geometry_part", 132, 9, struct_gbxmodel_geometry_part_runs},
	{"struct glow_definition", 340, 16, struct_glow_definition_runs},
	{"struct grenade_count_panel_definition", 192, 17, struct_grenade_count_panel_definition_runs},
	{"struct grenade_hud_interface_definition", 504, 28, struct_grenade_hud_interface_definition_runs},
	{"struct hs_global", 92, 2, struct_hs_global_runs},
	{"struct hs_reference", 40, 1, struct_hs_reference_runs},
	{"struct hs_script", 92, 2, struct_hs_script_runs},
	{"struct hs_source_file", 52, 1, struct_hs_source_file_runs},
	{"struct hud_absolute_placement_definition", 36, 2, struct_hud_absolute_placement_definition_runs},
	{"struct hud_globals_definition", 1104, 30, struct_hud_globals_definition_runs},
	{"struct hud_message_text_definition", 128, 1, struct_hud_message_text_definition_runs},
	{"struct hud_number_definition", 100, 3, struct_hud_number_definition_runs},
	{"struct hud_screen_effect_definition", 184, 9, struct_hud_screen_effect_definition_runs},
	{"struct hud_sound_definition", 56, 1, struct_hud_sound_definition_runs},
	{"struct hud_state_message_definition", 64, 2, struct_hud_state_message_definition_runs},
	{"struct hud_state_message_element", 2, 0, struct_hud_state_message_element_runs},
	{"struct hud_waypoint_arrow", 104, 3, struct_hud_waypoint_arrow_runs},
	{"struct icon_hud_element_definition", 16, 3, struct_icon_hud_element_definition_runs},
	{"struct item_collection_definition", 92, 3, struct_item_collection_definition_runs},
	{"struct item_definition", 776, 12, struct_item_definition_runs},
	{"struct item_permutation_definition", 84, 1, struct_item_permutation_definition_runs},
	{"struct leaf_map", 28, 1, struct_leaf_map_runs},
	{"struct leaf_portal", 24, 1, struct_leaf_portal_runs},
	{"struct lens_flare_definition", 240, 8, struct_lens_flare_definition_runs},
	{"struct lens_flare_reflection", 128, 9, struct_lens_flare_reflection_runs},
	{"struct light_definition", 352, 14, struct_light_definition_runs},
	{"struct light_fixture_placement", 88, 6, struct_light_fixture_placement_runs},
	{"struct light_volume_definition", 332, 8, struct_light_volume_definition_runs},
	{"struct light_volume_frame", 176, 1, struct_light_volume_frame_runs},
	{"struct lightning_definition", 264, 5, struct_lightning_definition_runs},
	{"struct lightning_marker_definition", 228, 2, struct_lightning_marker_definition_runs},
	{"struct lightning_shader", 180, 10, struct_lightning_shader_runs},
	{"struct looping_sound_definition", 84, 1, struct_looping_sound_definition_runs},
	{"struct looping_sound_detail", 104, 1, struct_looping_sound_detail_runs},
	{"struct looping_sound_track", 160, 1, struct_looping_sound_track_runs},
	{"struct machine_definition", 804, 12, struct_machine_definition_runs},
	{"struct machine_placement", 64, 6, struct_machine_placement_runs},
	{"struct map_leaf", 24, 1, struct_map_leaf_runs},
	{"struct map_leaf_face", 16, 1, struct_map_leaf_face_runs},
	{"struct mass_point_definition", 128, 4, struct_mass_point_definition_runs},
	{"struct material_definition", 884, 1, struct_material_definition_runs},
	{"struct material_effect", 28, 1, struct_material_effect_runs},
	{"struct material_effect_material", 48, 1, struct_material_effect_material_runs},
	{"struct material_effects_definition", 12, 1, struct_material_effects_definition_runs},
	{"struct meter_hud_element_definition", 104, 7, struct_meter_hud_element_definition_runs},
	{"struct metered_panel_definition", 240, 15, struct_metered_panel_definition_runs},
	{"struct model", 232, 3, struct_model_runs},
	{"struct model_geometry", 48, 1, struct_model_geometry_runs},
	{"struct model_geometry_part", 104, 8, struct_model_geometry_part_runs},
	{"struct model_marker", 64, 1, struct_model_marker_runs},
	{"struct model_marker_instance", 32, 1, struct_model_marker_instance_runs},
	{"struct model_node", 156, 2, struct_model_node_runs},
	{"struct model_region", 76, 1, struct_model_region_runs},
	{"struct model_region_permutation", 88, 5, struct_model_region_permutation_runs},
	{"struct model_region_permutation_marker", 80, 2, struct_model_region_permutation_marker_runs},
	{"struct model_shader_reference", 32, 3, struct_model_shader_reference_runs},
	{"struct model_triangle_element", 6, 1, struct_model_triangle_element_runs},
	{"struct model_vertex_compressed_element", 32, 0, struct_model_vertex_compressed_element_runs},
	{"struct model_vertex_uncompressed_element", 68, 0, struct_model_vertex_uncompressed_element_runs},
	{"struct motion_sensor_panel_definition", 240, 16, struct_motion_sensor_panel_definition_runs},
	{"struct move_position_definition", 80, 5, struct_move_position_definition_runs},
	{"struct multiplayer_scenario_description", 12, 1, struct_multiplayer_scenario_description_runs},
	{"struct multiplayer_scenario_description_entry", 68, 2, struct_multiplayer_scenario_description_entry_runs},
	{"struct multitexture_overlay_hud_element_definition", 480, 6, struct_multitexture_overlay_hud_element_definition_runs},
	{"struct multitexture_overlay_hud_element_effector_definition", 220, 5, struct_multitexture_overlay_hud_element_effector_definition_runs},
	{"struct object_attachment_definition", 72, 3, struct_object_attachment_definition_runs},
	{"struct object_change_color_definition", 44, 2, struct_object_change_color_definition_runs},
	{"struct object_change_color_permutation", 28, 1, struct_object_change_color_permutation_runs},
	{"struct object_definition", 380, 6, struct_object_definition_runs},
	{"struct object_definition_widget", 32, 1, struct_object_definition_widget_runs},
	{"struct object_function_definition", 360, 7, struct_object_function_definition_runs},
	{"struct optional_meter_hud_element", 104, 7, struct_optional_meter_hud_element_runs},
	{"struct optional_static_hud_element", 104, 8, struct_optional_static_hud_element_runs},
	{"struct particle_definition", 356, 13, struct_particle_definition_runs},
	{"struct particle_system_definition", 104, 3, struct_particle_system_definition_runs},
	{"struct particle_system_physics_constant", 4, 1, struct_particle_system_physics_constant_runs},
	{"struct particle_system_type", 128, 5, struct_particle_system_type_runs},
	{"struct particle_system_type_particle_state", 376, 13, struct_particle_system_type_particle_state_runs},
	{"struct particle_system_type_state", 192, 3, struct_particle_system_type_state_runs},
	{"struct pathfinding_sphere", 32, 2, struct_pathfinding_sphere_runs},
	{"struct physics_definition", 128, 1, struct_physics_definition_runs},
	{"struct platoon_definition", 172, 5, struct_platoon_definition_runs},
	{"struct platoon_rule", 8, 2, struct_platoon_rule_runs},
	{"struct player_starting_location", 52, 2, struct_player_starting_location_runs},
	{"struct point_physics_definition", 64, 1, struct_point_physics_definition_runs},
	{"struct powered_mass_point_definition", 128, 1, struct_powered_mass_point_definition_runs},
	{"struct powered_seat_definition", 68, 1, struct_powered_seat_definition_runs},
	{"struct predicted_resource", 8, 2, struct_predicted_resource_runs},
	{"struct projectile_definition", 588, 10, struct_projectile_definition_runs},
	{"struct projectile_material_response_definition", 160, 7, struct_projectile_material_response_definition_runs},
	{"struct recorded_animation_definition", 64, 2, struct_recorded_animation_definition_runs},
	{"struct render_lighting", 116, 5, struct_render_lighting_runs},
	{"struct scenario", 1456, 3, struct_scenario_runs},
	{"struct scenario_bsp_switch_trigger_volume", 8, 1, struct_scenario_bsp_switch_trigger_volume_runs},
	{"struct scenario_conversation_line", 124, 3, struct_scenario_conversation_line_runs},
	{"struct scenario_conversation_participant", 84, 3, struct_scenario_conversation_participant_runs},
	{"struct scenario_cutscene_camera_point", 104, 2, struct_scenario_cutscene_camera_point_runs},
	{"struct scenario_cutscene_flag", 92, 2, struct_scenario_cutscene_flag_runs},
	{"struct scenario_cutscene_title", 96, 4, struct_scenario_cutscene_title_runs},
	{"struct scenario_decal_palette_entry", 16, 1, struct_scenario_decal_palette_entry_runs},
	{"struct scenario_detail_object_collection_palette_entry", 48, 1, struct_scenario_detail_object_collection_palette_entry_runs},
	{"struct scenario_device_group", 52, 1, struct_scenario_device_group_runs},
	{"struct scenario_netgame_equipment", 144, 3, struct_scenario_netgame_equipment_runs},
	{"struct scenario_netgame_flag", 148, 3, struct_scenario_netgame_flag_runs},
	{"struct scenario_object_name", 36, 1, struct_scenario_object_name_runs},
	{"struct scenario_object_palette_entry", 48, 1, struct_scenario_object_palette_entry_runs},
	{"struct scenario_starting_equipment", 204, 3, struct_scenario_starting_equipment_runs},
	{"struct scenario_starting_profile", 104, 4, struct_scenario_starting_profile_runs},
	{"struct scenario_structure_bsp_reference", 32, 2, struct_scenario_structure_bsp_reference_runs},
	{"struct scenario_tag_reference", 16, 1, struct_scenario_tag_reference_runs},
	{"struct scenario_trigger_volume", 96, 2, struct_scenario_trigger_volume_runs},
	{"struct scenario_weapon_datum", 92, 7, struct_scenario_weapon_datum_runs},
	{"struct scenery_placement", 72, 5, struct_scenery_placement_runs},
	{"struct shader", 40, 3, struct_shader_runs},
	{"struct shader_effect_definition", 180, 10, struct_shader_effect_definition_runs},
	{"struct shader_environment_definition", 836, 29, struct_shader_environment_definition_runs},
	{"struct shader_layer", 16, 1, struct_shader_layer_runs},
	{"struct shader_model_definition", 440, 18, struct_shader_model_definition_runs},
	{"struct shader_texture_animation", 56, 6, struct_shader_texture_animation_runs},
	{"struct shader_transparent", 56, 2, struct_shader_transparent_runs},
	{"struct shader_transparent_chicago_definition", 108, 5, struct_shader_transparent_chicago_definition_runs},
	{"struct shader_transparent_chicago_extended_definition", 120, 5, struct_shader_transparent_chicago_extended_definition_runs},
	{"struct shader_transparent_chicago_map", 220, 9, struct_shader_transparent_chicago_map_runs},
	{"struct shader_transparent_generic_definition", 108, 5, struct_shader_transparent_generic_definition_runs},
	{"struct shader_transparent_generic_map", 100, 8, struct_shader_transparent_generic_map_runs},
	{"struct shader_transparent_generic_stage", 112, 3, struct_shader_transparent_generic_stage_runs},
	{"struct shader_transparent_glass_definition", 480, 8, struct_shader_transparent_glass_definition_runs},
	{"struct shader_transparent_meter_definition", 260, 4, struct_shader_transparent_meter_definition_runs},
	{"struct shader_transparent_plasma_definition", 332, 11, struct_shader_transparent_plasma_definition_runs},
	{"struct shader_transparent_water_definition", 320, 10, struct_shader_transparent_water_definition_runs},
	{"struct sky_animation", 36, 2, struct_sky_animation_runs},
	{"struct sky_definition", 208, 2, struct_sky_definition_runs},
	{"struct sky_light", 116, 1, struct_sky_light_runs},
	{"struct sky_shader_function", 36, 1, struct_sky_shader_function_runs},
	{"struct sound_definition", 164, 9, struct_sound_definition_runs},
	{"struct sound_environment_definition", 72, 3, struct_sound_environment_definition_runs},
	{"struct sound_permutation", 124, 3, struct_sound_permutation_runs},
	{"struct sound_pitch_range", 72, 5, struct_sound_pitch_range_runs},
	{"struct sound_scenery_placement", 40, 4, struct_sound_scenery_placement_runs},
	{"struct squad_definition", 232, 8, struct_squad_definition_runs},
	{"struct static_hud_element_definition", 104, 8, struct_static_hud_element_definition_runs},
	{"struct string_list", 12, 1, struct_string_list_runs},
	{"struct string_list_entry", 20, 1, struct_string_list_entry_runs},
	{"struct structure_background_sound_palette_entry", 116, 1, struct_structure_background_sound_palette_entry_runs},
	{"struct structure_breakable_surface", 48, 1, struct_structure_breakable_surface_runs},
	{"struct structure_bsp", 648, 5, struct_structure_bsp_runs},
	{"struct structure_cluster_portal", 64, 2, struct_structure_cluster_portal_runs},
	{"struct structure_cluster_schema", 104, 4, struct_structure_cluster_schema_runs},
	{"struct structure_collision_material", 20, 2, struct_structure_collision_material_runs},
	{"struct structure_detail_object_data", 64, 2, struct_structure_detail_object_data_runs},
	{"struct structure_fog_palette_entry", 136, 1, struct_structure_fog_palette_entry_runs},
	{"struct structure_fog_plane_schema", 32, 2, struct_structure_fog_plane_schema_runs},
	{"struct structure_fog_region", 40, 1, struct_structure_fog_region_runs},
	{"struct structure_leaf", 16, 2, struct_structure_leaf_runs},
	{"struct structure_lens_flare", 16, 1, struct_structure_lens_flare_runs},
	{"struct structure_lens_flare_marker", 16, 1, struct_structure_lens_flare_marker_runs},
	{"struct structure_lightmap", 32, 2, struct_structure_lightmap_runs},
	{"struct structure_material", 256, 11, struct_structure_material_runs},
	{"struct structure_mirror", 64, 1, struct_structure_mirror_runs},
	{"struct structure_portal_index", 2, 1, struct_structure_portal_index_runs},
	{"struct structure_runtime_decal", 16, 1, struct_structure_runtime_decal_runs},
	{"struct structure_sound_environment_palette_entry", 80, 1, struct_structure_sound_environment_palette_entry_runs},
	{"struct structure_subcluster", 36, 1, struct_structure_subcluster_runs},
	{"struct structure_surface", 6, 1, struct_structure_surface_runs},
	{"struct structure_surface_reference", 8, 1, struct_structure_surface_reference_runs},
	{"struct structure_weather_palette_entry", 240, 5, struct_structure_weather_palette_entry_runs},
	{"struct structure_weather_polyhedron", 32, 1, struct_structure_weather_polyhedron_runs},
	{"struct tag_reference_element", 16, 1, struct_tag_reference_element_runs},
	{"struct ui_widget_child_reference", 80, 3, struct_ui_widget_child_reference_runs},
	{"struct ui_widget_collection", 12, 1, struct_ui_widget_collection_runs},
	{"struct ui_widget_collection_widget", 16, 1, struct_ui_widget_collection_widget_runs},
	{"struct ui_widget_conditional_reference", 80, 3, struct_ui_widget_conditional_reference_runs},
	{"struct ui_widget_definition", 1004, 11, struct_ui_widget_definition_runs},
	{"struct ui_widget_event_handler_reference", 72, 3, struct_ui_widget_event_handler_reference_runs},
	{"struct ui_widget_game_data_input_reference", 36, 1, struct_ui_widget_game_data_input_reference_runs},
	{"struct ui_widget_search_and_replace_reference", 34, 1, struct_ui_widget_search_and_replace_reference_runs},
	{"struct unit_camera", 88, 1, struct_unit_camera_runs},
	{"struct unit_camera_track", 28, 1, struct_unit_camera_track_runs},
	{"struct unit_definition", 752, 17, struct_unit_definition_runs},
	{"struct unit_dialogue_variant_definition", 24, 2, struct_unit_dialogue_variant_definition_runs},
	{"struct unit_hud_interface_definition", 1388, 62, struct_unit_hud_interface_definition_runs},
	{"struct unit_hud_reference", 48, 1, struct_unit_hud_reference_runs},
	{"struct unit_initial_weapon", 36, 1, struct_unit_initial_weapon_runs},
	{"struct unit_placement", 120, 5, struct_unit_placement_runs},
	{"struct unit_seat", 284, 5, struct_unit_seat_runs},
	{"struct vehicle_animation", 116, 5, struct_vehicle_animation_runs},
	{"struct vehicle_definition", 1008, 24, struct_vehicle_definition_runs},
	{"struct vehicle_suspension", 20, 2, struct_vehicle_suspension_runs},
	{"struct vibrate_frequency_definition", 20, 3, struct_vibrate_frequency_definition_runs},
	{"struct virtual_keyboard_definition", 60, 1, struct_virtual_keyboard_definition_runs},
	{"struct virtual_keyboard_key", 80, 2, struct_virtual_keyboard_key_runs},
	{"struct water_ripple", 76, 3, struct_water_ripple_runs},
	{"struct weapon_ammunition_object", 28, 2, struct_weapon_ammunition_object_runs},
	{"struct weapon_definition", 1288, 20, struct_weapon_definition_runs},
	{"struct weapon_firing_effect", 132, 2, struct_weapon_firing_effect_runs},
	{"struct weapon_hud_crosshair_item", 108, 8, struct_weapon_hud_crosshair_item_runs},
	{"struct weapon_hud_crosshairs_element", 104, 2, struct_weapon_hud_crosshairs_element_runs},
	{"struct weapon_hud_element_header", 36, 2, struct_weapon_hud_element_header_runs},
	{"struct weapon_hud_interface_definition", 380, 9, struct_weapon_hud_interface_definition_runs},
	{"struct weapon_hud_meter_element", 180, 9, struct_weapon_hud_meter_element_runs},
	{"struct weapon_hud_number_element", 160, 12, struct_weapon_hud_number_element_runs},
	{"struct weapon_hud_overlay_definition", 28, 1, struct_weapon_hud_overlay_definition_runs},
	{"struct weapon_hud_overlay_item", 136, 8, struct_weapon_hud_overlay_item_runs},
	{"struct weapon_hud_overlays_element", 104, 2, struct_weapon_hud_overlays_element_runs},
	{"struct weapon_hud_static_element", 180, 10, struct_weapon_hud_static_element_runs},
	{"struct weapon_magazine_definition", 112, 7, struct_weapon_magazine_definition_runs},
	{"struct weapon_trigger_definition", 276, 7, struct_weapon_trigger_definition_runs},
	{"struct weather_particle_system_definition", 48, 1, struct_weather_particle_system_definition_runs},
	{"struct weather_particle_type_definition", 604, 11, struct_weather_particle_type_definition_runs},
	{"struct wind_definition", 64, 1, struct_wind_definition_runs},
	{"word", 2, 1, word_runs},
};

long const tag_swap_type_count = 324;
