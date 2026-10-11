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
static struct tag_swap_run const struct__biped_definition_runs[] = {{0, 4, 7}, {28, 2, 4}, {36, 4, 1}, {40, 5, 1}, {44, 4, 29}, {160, 5, 1}, {164, 4, 84}, {500, 2, 2}, {504, 4, 1}, {508, 5, 2}};
static struct tag_swap_run const struct__control_definition_runs[] = {{0, 2, 2}, {4, 4, 1}, {88, 4, 1}, {92, 5, 1}, {96, 4, 3}, {108, 5, 1}, {112, 4, 3}, {124, 5, 1}, {128, 4, 2}};
static struct tag_swap_run const struct__device_definition_runs[] = {{0, 4, 7}, {28, 2, 4}, {36, 4, 1}, {40, 5, 1}, {44, 4, 3}, {56, 5, 1}, {60, 4, 3}, {72, 5, 1}, {76, 4, 3}, {88, 5, 1}, {92, 4, 3}, {104, 5, 1}, {108, 4, 3}, {120, 5, 1}, {124, 4, 6}, {148, 5, 1}, {152, 4, 31}};
static struct tag_swap_run const struct__equipment_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 5, 1}, {16, 4, 2}};
static struct tag_swap_run const struct__item_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 1}, {12, 2, 2}, {16, 4, 4}, {32, 2, 4}, {40, 4, 42}, {208, 5, 1}, {212, 4, 3}, {224, 5, 1}, {228, 4, 35}, {368, 5, 1}, {372, 4, 3}, {384, 5, 1}, {388, 4, 2}};
static struct tag_swap_run const struct__machine_definition_runs[] = {{0, 2, 2}, {4, 4, 21}, {88, 2, 2}, {92, 4, 14}};
static struct tag_swap_run const struct__object_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}};
static struct tag_swap_run const struct__projectile_definition_runs[] = {{0, 4, 1}, {4, 2, 6}, {16, 4, 1}, {20, 5, 1}, {24, 4, 7}, {52, 5, 1}, {56, 4, 15}, {116, 2, 2}, {120, 4, 1}, {124, 5, 1}, {128, 4, 3}, {140, 5, 1}, {144, 4, 3}, {156, 5, 1}, {160, 4, 3}, {172, 5, 1}, {176, 4, 6}, {200, 5, 2}};
static struct tag_swap_run const struct__unit_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 2}, {16, 5, 1}, {20, 4, 2}, {28, 2, 4}, {36, 4, 2}, {108, 4, 4}, {124, 5, 2}, {132, 4, 20}, {212, 5, 1}, {216, 4, 2}, {224, 2, 2}, {228, 4, 11}, {272, 5, 1}, {276, 4, 2}, {284, 2, 2}, {288, 4, 4}, {304, 5, 2}, {312, 4, 1}, {316, 5, 2}, {324, 4, 1}, {328, 2, 4}, {336, 4, 1}, {340, 5, 2}, {348, 4, 1}, {352, 5, 2}, {360, 4, 1}, {364, 5, 2}};
static struct tag_swap_run const struct__weapon_definition_runs[] = {{0, 4, 1}, {36, 2, 6}, {48, 4, 2}, {56, 5, 1}, {60, 4, 13}, {112, 5, 1}, {116, 4, 3}, {128, 5, 1}, {132, 4, 3}, {144, 5, 1}, {148, 4, 3}, {160, 5, 1}, {164, 4, 5}, {184, 5, 1}, {188, 4, 5}, {208, 2, 2}, {212, 4, 8}, {244, 2, 2}, {248, 4, 9}, {284, 5, 1}, {288, 4, 3}, {300, 5, 1}, {304, 4, 10}, {344, 5, 1}, {348, 4, 3}, {360, 5, 1}, {364, 4, 4}, {380, 5, 1}, {384, 4, 3}, {396, 5, 1}, {400, 4, 3}, {412, 5, 1}, {416, 4, 3}, {428, 5, 1}, {432, 4, 3}, {444, 5, 2}, {452, 4, 5}, {472, 2, 2}, {476, 4, 1}, {480, 5, 2}, {488, 4, 1}, {492, 5, 2}, {500, 4, 1}, {504, 5, 2}};
static struct tag_swap_run const struct_actor_definition_runs[] = {{0, 4, 5}, {20, 2, 2}, {24, 4, 74}, {320, 5, 1}, {324, 4, 70}, {604, 5, 1}, {608, 4, 2}, {616, 2, 4}, {624, 4, 2}, {632, 2, 2}, {636, 4, 10}, {676, 2, 2}, {680, 4, 20}, {760, 2, 2}, {764, 4, 22}, {852, 2, 2}, {856, 4, 89}, {1212, 5, 1}, {1216, 4, 14}};
static struct tag_swap_run const struct_actor_starting_location_runs[] = {{0, 4, 4}, {16, 2, 1}, {20, 2, 4}};
static struct tag_swap_run const struct_actor_variant_change_colors_runs[] = {{0, 4, 8}};
static struct tag_swap_run const struct_actor_variant_definition_runs[] = {{0, 4, 2}, {8, 5, 1}, {12, 4, 3}, {24, 5, 1}, {28, 4, 3}, {40, 5, 1}, {44, 4, 8}, {76, 2, 2}, {80, 4, 6}, {104, 5, 1}, {108, 4, 58}, {340, 2, 2}, {344, 4, 10}, {384, 2, 4}, {392, 4, 15}, {452, 5, 1}, {456, 4, 2}, {464, 2, 2}, {468, 4, 3}, {480, 2, 2}, {484, 4, 10}, {524, 2, 2}, {528, 4, 8}, {560, 5, 2}};
static struct tag_swap_run const struct_ai_animation_reference_definition_runs[] = {{32, 4, 1}, {36, 5, 1}, {40, 4, 5}};
static struct tag_swap_run const struct_ai_command_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 8}, {28, 4, 1}};
static struct tag_swap_run const struct_ai_command_list_definition_runs[] = {{32, 4, 3}, {44, 2, 2}, {48, 4, 1}, {52, 5, 2}, {60, 4, 1}, {64, 5, 2}, {72, 4, 1}, {76, 5, 2}, {84, 4, 1}, {88, 5, 2}};
static struct tag_swap_run const struct_ai_command_point_definition_runs[] = {{0, 4, 5}};
static struct tag_swap_run const struct_ai_conversation_runs[] = {{32, 2, 2}, {36, 4, 2}, {80, 4, 1}, {84, 5, 2}, {92, 4, 1}, {96, 5, 2}, {104, 4, 1}, {108, 5, 2}};
static struct tag_swap_run const struct_ai_recording_reference_definition_runs[] = {{32, 4, 2}};
static struct tag_swap_run const struct_ai_script_reference_definition_runs[] = {{32, 4, 2}};
static struct tag_swap_run const struct_animation_runs[] = {{32, 2, 4}, {40, 4, 1}, {44, 2, 2}, {48, 4, 1}, {52, 2, 6}, {66, 2, 1}, {68, 4, 4}, {84, 5, 2}, {92, 4, 15}, {152, 5, 2}, {160, 4, 3}, {172, 5, 2}};
static struct tag_swap_run const struct_animation_graph_runs[] = {{0, 4, 1}, {4, 5, 2}, {12, 4, 1}, {16, 5, 2}, {24, 4, 1}, {28, 5, 2}, {36, 4, 1}, {40, 5, 2}, {48, 4, 1}, {52, 5, 2}, {60, 4, 1}, {64, 5, 2}, {72, 4, 1}, {76, 5, 2}, {84, 4, 1}, {88, 5, 2}, {96, 4, 1}, {100, 2, 2}, {104, 4, 1}, {108, 5, 2}, {116, 4, 1}, {120, 5, 2}};
static struct tag_swap_run const struct_animation_graph_animation_index_runs[] = {{0, 2, 1}};
static struct tag_swap_run const struct_animation_graph_device_animations_runs[] = {{0, 4, 22}, {88, 5, 2}};
static struct tag_swap_run const struct_animation_graph_ik_point_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_animation_graph_node_runs[] = {{32, 2, 4}, {40, 4, 6}};
static struct tag_swap_run const struct_animation_graph_object_overlay_runs[] = {{0, 2, 4}, {8, 4, 3}};
static struct tag_swap_run const struct_animation_graph_sound_reference_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}};
static struct tag_swap_run const struct_animation_graph_unit_seat_runs[] = {{32, 4, 2}, {40, 2, 2}, {44, 4, 2}, {52, 2, 2}, {56, 4, 3}, {68, 5, 2}, {76, 4, 1}, {80, 5, 2}, {88, 4, 1}, {92, 5, 2}};
static struct tag_swap_run const struct_animation_graph_weapon_animations_runs[] = {{0, 4, 5}, {20, 5, 2}};
static struct tag_swap_run const struct_animation_graph_weapon_class_runs[] = {{96, 4, 2}, {104, 2, 2}, {108, 4, 2}, {116, 2, 2}, {120, 4, 9}, {156, 5, 2}, {164, 4, 1}, {168, 5, 2}, {176, 4, 1}, {180, 5, 2}};
static struct tag_swap_run const struct_animation_graph_weapon_type_runs[] = {{32, 4, 5}, {52, 5, 2}};
static struct tag_swap_run const struct_antenna_definition_runs[] = {{32, 4, 1}, {36, 5, 1}, {40, 4, 3}, {52, 5, 1}, {56, 4, 36}, {200, 5, 2}};
static struct tag_swap_run const struct_antenna_vertex_definition_runs[] = {{0, 4, 10}, {40, 2, 2}, {44, 4, 21}};
static struct tag_swap_run const struct_auxilary_meter_definition_runs[] = {{0, 2, 2}, {4, 4, 4}, {20, 2, 2}, {24, 4, 2}, {32, 2, 2}, {36, 4, 6}, {60, 5, 1}, {64, 4, 6}, {88, 2, 2}, {92, 4, 2}, {100, 2, 4}, {108, 4, 1}, {112, 5, 2}, {120, 4, 1}, {124, 2, 2}, {128, 4, 2}, {136, 2, 2}, {140, 4, 6}, {164, 5, 1}, {168, 4, 6}, {194, 2, 1}, {198, 2, 1}, {200, 4, 4}, {216, 5, 2}, {224, 4, 25}};
static struct tag_swap_run const struct_auxilary_overlay_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 6}, {40, 5, 1}, {44, 4, 6}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 1}, {92, 5, 2}, {100, 4, 1}, {104, 2, 2}, {108, 4, 6}};
static struct tag_swap_run const struct_biped_contact_point_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_biped_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 1}, {384, 2, 2}, {388, 4, 2}, {396, 5, 1}, {400, 4, 2}, {408, 2, 4}, {416, 4, 2}, {488, 4, 4}, {504, 5, 2}, {512, 4, 20}, {592, 5, 1}, {596, 4, 2}, {604, 2, 2}, {608, 4, 11}, {652, 5, 1}, {656, 4, 2}, {664, 2, 2}, {668, 4, 4}, {684, 5, 2}, {692, 4, 1}, {696, 5, 2}, {704, 4, 1}, {708, 2, 4}, {716, 4, 1}, {720, 5, 2}, {728, 4, 1}, {732, 5, 2}, {740, 4, 1}, {744, 5, 2}, {752, 4, 7}, {780, 2, 4}, {788, 4, 1}, {792, 5, 1}, {796, 4, 29}, {912, 5, 1}, {916, 4, 84}, {1252, 2, 2}, {1256, 4, 1}, {1260, 5, 2}};
static struct tag_swap_run const struct_bitmap_data_runs[] = {{0, 4, 1}, {4, 2, 10}, {24, 4, 4}, {40, 5, 2}};
static struct tag_swap_run const struct_bitmap_group_runs[] = {{0, 2, 4}, {8, 4, 3}, {20, 2, 4}, {28, 4, 3}, {40, 5, 2}, {48, 4, 3}, {60, 5, 2}, {68, 4, 2}, {76, 2, 4}, {84, 4, 1}, {88, 5, 2}, {96, 4, 1}, {100, 5, 2}};
static struct tag_swap_run const struct_bitmap_group_sequence_runs[] = {{32, 2, 2}, {36, 4, 5}, {56, 5, 2}};
static struct tag_swap_run const struct_bitmap_group_sprite_runs[] = {{0, 2, 2}, {4, 4, 7}};
static struct tag_swap_run const struct_breakable_surface_particle_effect_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 30}};
static struct tag_swap_run const struct_bsp2d_node_runs[] = {{0, 4, 5}};
static struct tag_swap_run const struct_bsp2d_reference_runs[] = {{0, 4, 2}};
static struct tag_swap_run const struct_bsp3d_node_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_camera_track_control_point_runs[] = {{0, 4, 15}};
static struct tag_swap_run const struct_camera_track_definition_runs[] = {{0, 4, 2}, {8, 5, 2}, {16, 4, 8}};
static struct tag_swap_run const struct_collision_bsp_runs[] = {{0, 4, 1}, {4, 5, 2}, {12, 4, 1}, {16, 5, 2}, {24, 4, 1}, {28, 5, 2}, {36, 4, 1}, {40, 5, 2}, {48, 4, 1}, {52, 5, 2}, {60, 4, 1}, {64, 5, 2}, {72, 4, 1}, {76, 5, 2}, {84, 4, 1}, {88, 5, 2}};
static struct tag_swap_run const struct_collision_edge_runs[] = {{0, 4, 6}};
static struct tag_swap_run const struct_collision_leaf_runs[] = {{0, 2, 2}, {4, 4, 1}};
static struct tag_swap_run const struct_collision_model_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 27}, {116, 5, 1}, {120, 4, 4}, {136, 5, 1}, {140, 4, 4}, {156, 5, 1}, {160, 4, 3}, {172, 5, 1}, {176, 4, 4}, {192, 5, 1}, {196, 4, 3}, {208, 2, 2}, {212, 4, 6}, {236, 2, 2}, {240, 4, 39}, {396, 5, 1}, {400, 4, 3}, {412, 5, 1}, {416, 4, 3}, {428, 5, 1}, {432, 4, 34}, {568, 5, 2}, {576, 4, 1}, {580, 5, 2}, {588, 4, 1}, {592, 5, 2}, {600, 4, 11}, {644, 5, 2}, {652, 4, 1}, {656, 5, 2}};
static struct tag_swap_run const struct_collision_node_runs[] = {{32, 2, 4}, {40, 4, 2}, {48, 2, 2}, {52, 4, 1}, {56, 5, 2}};
static struct tag_swap_run const struct_collision_surface_runs[] = {{0, 4, 2}, {10, 2, 1}};
static struct tag_swap_run const struct_collision_vertex_runs[] = {{0, 4, 4}};
static struct tag_swap_run const struct_color_table_color_runs[] = {{32, 4, 4}};
static struct tag_swap_run const struct_color_table_definition_runs[] = {{0, 4, 1}, {4, 5, 2}};
static struct tag_swap_run const struct_continuous_damage_effect_definition_runs[] = {{0, 4, 22}, {88, 2, 2}, {92, 4, 50}, {292, 2, 2}, {296, 4, 54}};
static struct tag_swap_run const struct_contrail_definition_runs[] = {{0, 2, 2}, {4, 4, 5}, {24, 2, 2}, {28, 4, 6}, {52, 5, 1}, {56, 4, 2}, {64, 2, 2}, {68, 4, 16}, {132, 2, 2}, {136, 4, 7}, {164, 2, 4}, {176, 2, 1}, {312, 4, 1}, {316, 5, 2}};
static struct tag_swap_run const struct_contrail_point_state_runs[] = {{0, 4, 5}, {20, 5, 1}, {24, 4, 20}};
static struct tag_swap_run const struct_control_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 7}, {408, 2, 4}, {416, 4, 1}, {420, 5, 1}, {424, 4, 3}, {436, 5, 1}, {440, 4, 3}, {452, 5, 1}, {456, 4, 3}, {468, 5, 1}, {472, 4, 3}, {484, 5, 1}, {488, 4, 3}, {500, 5, 1}, {504, 4, 6}, {528, 5, 1}, {532, 4, 31}, {656, 2, 2}, {660, 4, 1}, {744, 4, 1}, {748, 5, 1}, {752, 4, 3}, {764, 5, 1}, {768, 4, 3}, {780, 5, 1}, {784, 4, 2}};
static struct tag_swap_run const struct_control_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}, {40, 2, 2}, {44, 4, 1}, {48, 2, 3}};
static struct tag_swap_run const struct_damage_definition_runs[] = {{0, 2, 2}, {4, 4, 54}};
static struct tag_swap_run const struct_damage_effect_definition_runs[] = {{0, 4, 9}, {36, 2, 2}, {40, 4, 4}, {56, 2, 2}, {60, 4, 10}, {100, 2, 2}, {104, 4, 4}, {120, 2, 2}, {124, 4, 8}, {156, 2, 2}, {160, 4, 12}, {208, 2, 2}, {212, 4, 5}, {232, 2, 2}, {236, 4, 11}, {280, 5, 1}, {284, 4, 42}, {452, 2, 2}, {456, 4, 54}};
static struct tag_swap_run const struct_damage_permutation_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_damage_region_runs[] = {{32, 4, 7}, {60, 5, 1}, {64, 4, 3}, {76, 5, 2}};
static struct tag_swap_run const struct_damage_resistance_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 27}, {116, 5, 1}, {120, 4, 4}, {136, 5, 1}, {140, 4, 4}, {156, 5, 1}, {160, 4, 3}, {172, 5, 1}, {176, 4, 4}, {192, 5, 1}, {196, 4, 3}, {208, 2, 2}, {212, 4, 6}, {236, 2, 2}, {240, 4, 39}, {396, 5, 1}, {400, 4, 3}, {412, 5, 1}, {416, 4, 3}, {428, 5, 1}, {432, 4, 34}, {568, 5, 2}, {576, 4, 1}, {580, 5, 2}, {588, 4, 1}, {592, 5, 2}};
static struct tag_swap_run const struct_damage_resistance_material_runs[] = {{32, 4, 1}, {36, 2, 2}, {40, 4, 8}};
static struct tag_swap_run const struct_decal_definition_runs[] = {{0, 2, 4}, {8, 4, 1}, {12, 5, 1}, {16, 4, 18}, {88, 2, 2}, {92, 4, 14}, {148, 2, 2}, {152, 4, 7}, {180, 2, 8}, {196, 4, 6}, {220, 5, 1}, {224, 4, 8}, {256, 2, 2}, {260, 4, 2}};
static struct tag_swap_run const struct_detail_object_runs[] = {{4, 2, 1}};
static struct tag_swap_run const struct_detail_object_cell_definition_runs[] = {{0, 2, 4}, {8, 4, 6}};
static struct tag_swap_run const struct_detail_object_collection_definition_runs[] = {{0, 2, 2}, {4, 4, 13}, {56, 5, 1}, {60, 4, 3}, {72, 5, 2}, {80, 4, 12}};
static struct tag_swap_run const struct_detail_object_type_definition_runs[] = {{36, 4, 7}};
static struct tag_swap_run const struct_device_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 7}, {408, 2, 4}, {416, 4, 1}, {420, 5, 1}, {424, 4, 3}, {436, 5, 1}, {440, 4, 3}, {452, 5, 1}, {456, 4, 3}, {468, 5, 1}, {472, 4, 3}, {484, 5, 1}, {488, 4, 3}, {500, 5, 1}, {504, 4, 6}, {528, 5, 1}, {532, 4, 31}};
static struct tag_swap_run const struct_dialogue_definition_runs[] = {{0, 2, 2}, {4, 4, 4}, {20, 5, 1}, {24, 4, 3}, {36, 5, 1}, {40, 4, 3}, {52, 5, 1}, {56, 4, 3}, {68, 5, 1}, {72, 4, 3}, {84, 5, 1}, {88, 4, 3}, {100, 5, 1}, {104, 4, 3}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 3}, {180, 5, 1}, {184, 4, 3}, {196, 5, 1}, {200, 4, 3}, {212, 5, 1}, {216, 4, 3}, {228, 5, 1}, {232, 4, 3}, {244, 5, 1}, {248, 4, 3}, {260, 5, 1}, {264, 4, 3}, {276, 5, 1}, {280, 4, 3}, {292, 5, 1}, {296, 4, 3}, {308, 5, 1}, {312, 4, 3}, {324, 5, 1}, {328, 4, 3}, {340, 5, 1}, {344, 4, 3}, {356, 5, 1}, {360, 4, 3}, {372, 5, 1}, {376, 4, 3}, {388, 5, 1}, {392, 4, 3}, {404, 5, 1}, {408, 4, 3}, {420, 5, 1}, {424, 4, 3}, {436, 5, 1}, {440, 4, 3}, {452, 5, 1}, {456, 4, 3}, {468, 5, 1}, {472, 4, 3}, {484, 5, 1}, {488, 4, 3}, {500, 5, 1}, {504, 4, 3}, {516, 5, 1}, {520, 4, 3}, {532, 5, 1}, {536, 4, 3}, {548, 5, 1}, {552, 4, 3}, {564, 5, 1}, {568, 4, 3}, {580, 5, 1}, {584, 4, 3}, {596, 5, 1}, {600, 4, 3}, {612, 5, 1}, {616, 4, 3}, {628, 5, 1}, {632, 4, 3}, {644, 5, 1}, {648, 4, 3}, {660, 5, 1}, {664, 4, 3}, {676, 5, 1}, {680, 4, 3}, {692, 5, 1}, {696, 4, 3}, {708, 5, 1}, {712, 4, 3}, {724, 5, 1}, {728, 4, 3}, {740, 5, 1}, {744, 4, 3}, {756, 5, 1}, {760, 4, 3}, {772, 5, 1}, {776, 4, 3}, {788, 5, 1}, {792, 4, 3}, {804, 5, 1}, {808, 4, 3}, {820, 5, 1}, {824, 4, 3}, {836, 5, 1}, {840, 4, 3}, {852, 5, 1}, {856, 4, 3}, {868, 5, 1}, {872, 4, 3}, {884, 5, 1}, {888, 4, 3}, {900, 5, 1}, {904, 4, 3}, {916, 5, 1}, {920, 4, 3}, {932, 5, 1}, {936, 4, 3}, {948, 5, 1}, {952, 4, 3}, {964, 5, 1}, {968, 4, 3}, {980, 5, 1}, {984, 4, 3}, {996, 5, 1}, {1000, 4, 3}, {1012, 5, 1}, {1016, 4, 3}, {1028, 5, 1}, {1032, 4, 3}, {1044, 5, 1}, {1048, 4, 3}, {1060, 5, 1}, {1064, 4, 3}, {1076, 5, 1}, {1080, 4, 3}, {1092, 5, 1}, {1096, 4, 3}, {1108, 5, 1}, {1112, 4, 3}, {1124, 5, 1}, {1128, 4, 3}, {1140, 5, 1}, {1144, 4, 3}, {1156, 5, 1}, {1160, 4, 3}, {1172, 5, 1}, {1176, 4, 3}, {1188, 5, 1}, {1192, 4, 3}, {1204, 5, 1}, {1208, 4, 3}, {1220, 5, 1}, {1224, 4, 3}, {1236, 5, 1}, {1240, 4, 3}, {1252, 5, 1}, {1256, 4, 3}, {1268, 5, 1}, {1272, 4, 3}, {1284, 5, 1}, {1288, 4, 3}, {1300, 5, 1}, {1304, 4, 3}, {1316, 5, 1}, {1320, 4, 3}, {1332, 5, 1}, {1336, 4, 3}, {1348, 5, 1}, {1352, 4, 3}, {1364, 5, 1}, {1368, 4, 3}, {1380, 5, 1}, {1384, 4, 3}, {1396, 5, 1}, {1400, 4, 3}, {1412, 5, 1}, {1416, 4, 3}, {1428, 5, 1}, {1432, 4, 3}, {1444, 5, 1}, {1448, 4, 3}, {1460, 5, 1}, {1464, 4, 3}, {1476, 5, 1}, {1480, 4, 3}, {1492, 5, 1}, {1496, 4, 3}, {1508, 5, 1}, {1512, 4, 3}, {1524, 5, 1}, {1528, 4, 3}, {1540, 5, 1}, {1544, 4, 3}, {1556, 5, 1}, {1560, 4, 3}, {1572, 5, 1}, {1576, 4, 3}, {1588, 5, 1}, {1592, 4, 3}, {1604, 5, 1}, {1608, 4, 3}, {1620, 5, 1}, {1624, 4, 3}, {1636, 5, 1}, {1640, 4, 3}, {1652, 5, 1}, {1656, 4, 3}, {1668, 5, 1}, {1672, 4, 3}, {1684, 5, 1}, {1688, 4, 3}, {1700, 5, 1}, {1704, 4, 3}, {1716, 5, 1}, {1720, 4, 3}, {1732, 5, 1}, {1736, 4, 3}, {1748, 5, 1}, {1752, 4, 3}, {1764, 5, 1}, {1768, 4, 3}, {1780, 5, 1}, {1784, 4, 3}, {1796, 5, 1}, {1800, 4, 3}, {1812, 5, 1}, {1816, 4, 3}, {1828, 5, 1}, {1832, 4, 3}, {1844, 5, 1}, {1848, 4, 3}, {1860, 5, 1}, {1864, 4, 3}, {1876, 5, 1}, {1880, 4, 3}, {1892, 5, 1}, {1896, 4, 3}, {1908, 5, 1}, {1912, 4, 3}, {1924, 5, 1}, {1928, 4, 3}, {1940, 5, 1}, {1944, 4, 3}, {1956, 5, 1}, {1960, 4, 3}, {1972, 5, 1}, {1976, 4, 3}, {1988, 5, 1}, {1992, 4, 3}, {2004, 5, 1}, {2008, 4, 3}, {2020, 5, 1}, {2024, 4, 3}, {2036, 5, 1}, {2040, 4, 3}, {2052, 5, 1}, {2056, 4, 3}, {2068, 5, 1}, {2072, 4, 3}, {2084, 5, 1}, {2088, 4, 3}, {2100, 5, 1}, {2104, 4, 3}, {2116, 5, 1}, {2120, 4, 3}, {2132, 5, 1}, {2136, 4, 3}, {2148, 5, 1}, {2152, 4, 3}, {2164, 5, 1}, {2168, 4, 3}, {2180, 5, 1}, {2184, 4, 3}, {2196, 5, 1}, {2200, 4, 3}, {2212, 5, 1}, {2216, 4, 3}, {2228, 5, 1}, {2232, 4, 3}, {2244, 5, 1}, {2248, 4, 3}, {2260, 5, 1}, {2264, 4, 3}, {2276, 5, 1}, {2280, 4, 3}, {2292, 5, 1}, {2296, 4, 3}, {2308, 5, 1}, {2312, 4, 3}, {2324, 5, 1}, {2328, 4, 3}, {2340, 5, 1}, {2344, 4, 3}, {2356, 5, 1}, {2360, 4, 3}, {2372, 5, 1}, {2376, 4, 3}, {2388, 5, 1}, {2392, 4, 3}, {2404, 5, 1}, {2408, 4, 3}, {2420, 5, 1}, {2424, 4, 3}, {2436, 5, 1}, {2440, 4, 3}, {2452, 5, 1}, {2456, 4, 3}, {2468, 5, 1}, {2472, 4, 3}, {2484, 5, 1}, {2488, 4, 3}, {2500, 5, 1}, {2504, 4, 3}, {2516, 5, 1}, {2520, 4, 3}, {2532, 5, 1}, {2536, 4, 3}, {2548, 5, 1}, {2552, 4, 3}, {2564, 5, 1}, {2568, 4, 3}, {2580, 5, 1}, {2584, 4, 3}, {2596, 5, 1}, {2600, 4, 3}, {2612, 5, 1}, {2616, 4, 3}, {2628, 5, 1}, {2632, 4, 3}, {2644, 5, 1}, {2648, 4, 3}, {2660, 5, 1}, {2664, 4, 3}, {2676, 5, 1}, {2680, 4, 3}, {2692, 5, 1}, {2696, 4, 3}, {2708, 5, 1}, {2712, 4, 3}, {2724, 5, 1}, {2728, 4, 3}, {2740, 5, 1}, {2744, 4, 3}, {2756, 5, 1}, {2760, 4, 3}, {2772, 5, 1}, {2776, 4, 3}, {2788, 5, 1}, {2792, 4, 3}, {2804, 5, 1}, {2808, 4, 3}, {2820, 5, 1}, {2824, 4, 3}, {2836, 5, 1}, {2840, 4, 3}, {2852, 5, 1}, {2856, 4, 3}, {2868, 5, 1}, {2872, 4, 3}, {2884, 5, 1}, {2888, 4, 3}, {2900, 5, 1}, {2904, 4, 3}, {2916, 5, 1}, {2920, 4, 3}, {2932, 5, 1}, {2936, 4, 3}, {2948, 5, 1}, {2952, 4, 3}, {2964, 5, 1}, {2968, 4, 3}, {2980, 5, 1}, {2984, 4, 3}, {2996, 5, 1}, {3000, 4, 3}, {3012, 5, 1}, {3016, 4, 3}, {3028, 5, 1}, {3032, 4, 3}, {3044, 5, 1}, {3048, 4, 3}, {3060, 5, 1}, {3064, 4, 3}, {3076, 5, 1}, {3080, 4, 3}, {3092, 5, 1}, {3096, 4, 3}, {3108, 5, 1}, {3112, 4, 3}, {3124, 5, 1}, {3128, 4, 3}, {3140, 5, 1}, {3144, 4, 3}, {3156, 5, 1}, {3160, 4, 3}, {3172, 5, 1}, {3176, 4, 3}, {3188, 5, 1}, {3192, 4, 3}, {3204, 5, 1}, {3208, 4, 3}, {3220, 5, 1}, {3224, 4, 3}, {3236, 5, 1}, {3240, 4, 3}, {3252, 5, 1}, {3256, 4, 3}, {3268, 5, 1}, {3272, 4, 3}, {3284, 5, 1}, {3288, 4, 3}, {3300, 5, 1}, {3304, 4, 3}, {3316, 5, 1}, {3320, 4, 3}, {3332, 5, 1}, {3336, 4, 3}, {3348, 5, 1}, {3352, 4, 3}, {3364, 5, 1}, {3368, 4, 3}, {3380, 5, 1}, {3384, 4, 3}, {3396, 5, 1}, {3400, 4, 3}, {3412, 5, 1}, {3416, 4, 3}, {3428, 5, 1}, {3432, 4, 3}, {3444, 5, 1}, {3448, 4, 3}, {3460, 5, 1}, {3464, 4, 3}, {3476, 5, 1}, {3480, 4, 3}, {3492, 5, 1}, {3496, 4, 3}, {3508, 5, 1}, {3512, 4, 3}, {3524, 5, 1}, {3528, 4, 3}, {3540, 5, 1}, {3544, 4, 3}, {3556, 5, 1}, {3560, 4, 3}, {3572, 5, 1}, {3576, 4, 3}, {3588, 5, 1}, {3592, 4, 3}, {3604, 5, 1}, {3608, 4, 3}, {3620, 5, 1}, {3624, 4, 3}, {3636, 5, 1}, {3640, 4, 3}, {3652, 5, 1}, {3656, 4, 3}, {3668, 5, 1}, {3672, 4, 3}, {3684, 5, 1}, {3688, 4, 3}, {3700, 5, 1}, {3704, 4, 3}, {3716, 5, 1}, {3720, 4, 3}, {3732, 5, 1}, {3736, 4, 3}, {3748, 5, 1}, {3752, 4, 3}, {3764, 5, 1}, {3768, 4, 3}, {3780, 5, 1}, {3784, 4, 3}, {3796, 5, 1}, {3800, 4, 3}, {3812, 5, 1}, {3816, 4, 3}, {3828, 5, 1}, {3832, 4, 3}, {3844, 5, 1}, {3848, 4, 3}, {3860, 5, 1}, {3864, 4, 3}, {3876, 5, 1}, {3880, 4, 3}, {3892, 5, 1}, {3896, 4, 3}, {3908, 5, 1}, {3912, 4, 3}, {3924, 5, 1}, {3928, 4, 3}, {3940, 5, 1}, {3944, 4, 3}, {3956, 5, 1}, {3960, 4, 3}, {3972, 5, 1}, {3976, 4, 3}, {3988, 5, 1}, {3992, 4, 3}, {4004, 5, 1}, {4008, 4, 3}, {4020, 5, 1}, {4024, 4, 3}, {4036, 5, 1}, {4040, 4, 3}, {4052, 5, 1}, {4056, 4, 3}, {4068, 5, 1}, {4072, 4, 3}, {4084, 5, 1}, {4088, 4, 3}, {4100, 5, 1}, {4104, 4, 2}};
static struct tag_swap_run const struct_effect_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 9}, {44, 5, 2}, {52, 4, 1}, {56, 5, 2}};
static struct tag_swap_run const struct_effect_event_definition_runs[] = {{0, 4, 12}, {48, 5, 2}, {56, 4, 1}, {60, 5, 2}};
static struct tag_swap_run const struct_effect_location_definition_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_effect_part_definition_runs[] = {{0, 2, 4}, {8, 4, 5}, {28, 5, 1}, {32, 4, 18}};
static struct tag_swap_run const struct_effect_particles_definition_runs[] = {{0, 2, 6}, {12, 4, 19}, {88, 5, 1}, {92, 4, 3}, {104, 2, 4}, {112, 4, 30}};
static struct tag_swap_run const struct_encounter_definition_runs[] = {{32, 4, 1}, {36, 2, 4}, {44, 4, 20}, {124, 2, 2}, {128, 4, 1}, {132, 5, 2}, {140, 4, 1}, {144, 5, 2}, {152, 4, 1}, {156, 5, 2}, {164, 4, 1}, {168, 5, 2}};
static struct tag_swap_run const struct_encounter_player_starting_location_runs[] = {{0, 4, 4}, {16, 2, 6}};
static struct tag_swap_run const struct_equipment_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 1}, {384, 2, 2}, {388, 4, 1}, {392, 2, 2}, {396, 4, 4}, {412, 2, 4}, {420, 4, 42}, {588, 5, 1}, {592, 4, 3}, {604, 5, 1}, {608, 4, 35}, {748, 5, 1}, {752, 4, 3}, {764, 5, 1}, {768, 4, 2}, {776, 2, 2}, {780, 4, 2}, {788, 5, 1}, {792, 4, 2}};
static struct tag_swap_run const struct_equipment_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}};
static struct tag_swap_run const struct_firing_position_definition_runs[] = {{0, 4, 3}, {12, 2, 2}, {20, 4, 1}};
static struct tag_swap_run const struct_flag_attachment_point_runs[] = {{0, 2, 2}, {4, 4, 4}};
static struct tag_swap_run const struct_flag_definition_runs[] = {{0, 4, 1}, {4, 2, 6}, {16, 4, 3}, {28, 5, 1}, {32, 4, 3}, {44, 5, 1}, {48, 4, 6}, {72, 5, 1}, {76, 4, 3}, {88, 5, 2}};
static struct tag_swap_run const struct_fog_definition_full_runs[] = {{0, 2, 2}, {4, 4, 1}, {88, 4, 1}, {96, 4, 1}, {104, 4, 1}, {116, 4, 4}, {132, 2, 2}, {136, 4, 5}, {160, 4, 4}, {184, 4, 2}, {192, 5, 1}, {196, 4, 10}, {244, 4, 1}, {248, 5, 1}, {252, 4, 3}, {264, 5, 1}, {268, 4, 2}};
static struct tag_swap_run const struct_font_character_runs[] = {{0, 2, 8}, {16, 4, 1}};
static struct tag_swap_run const struct_font_character_index_runs[] = {{0, 2, 1}};
static struct tag_swap_run const struct_font_character_table_runs[] = {{0, 4, 1}, {4, 5, 2}};
static struct tag_swap_run const struct_font_header_runs[] = {{0, 4, 1}, {4, 2, 4}, {12, 4, 10}, {52, 5, 2}, {60, 4, 1}, {64, 5, 1}, {68, 4, 3}, {80, 5, 1}, {84, 4, 3}, {96, 5, 1}, {100, 4, 3}, {112, 5, 1}, {116, 4, 3}, {128, 5, 2}, {136, 4, 3}, {148, 5, 2}};
static struct tag_swap_run const struct_game_globals_runs[] = {{0, 4, 63}, {252, 5, 2}, {260, 4, 1}, {264, 5, 2}, {272, 4, 1}, {276, 5, 2}, {284, 4, 1}, {288, 5, 2}, {296, 4, 1}, {300, 5, 2}, {308, 4, 1}, {312, 5, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 1}, {384, 5, 2}, {392, 4, 1}, {396, 5, 2}, {404, 4, 1}, {408, 5, 2}, {416, 4, 1}, {420, 5, 2}};
static struct tag_swap_run const struct_game_globals_difficulty_information_runs[] = {{0, 4, 161}};
static struct tag_swap_run const struct_game_globals_falling_damage_runs[] = {{0, 4, 5}, {20, 5, 1}, {24, 4, 6}, {48, 5, 1}, {52, 4, 3}, {64, 5, 1}, {68, 4, 3}, {80, 5, 1}, {84, 4, 3}, {96, 5, 1}, {100, 4, 3}, {112, 5, 1}, {116, 4, 9}};
static struct tag_swap_run const struct_game_globals_first_person_interface_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 3}, {36, 5, 1}, {40, 4, 2}, {48, 2, 2}, {52, 4, 1}, {56, 5, 1}, {60, 4, 2}, {68, 2, 2}, {72, 4, 1}, {76, 5, 1}, {80, 4, 3}, {92, 5, 1}, {96, 4, 24}};
static struct tag_swap_run const struct_game_globals_grenade_runs[] = {{0, 2, 2}, {4, 4, 1}, {8, 5, 1}, {12, 4, 3}, {24, 5, 1}, {28, 4, 3}, {40, 5, 1}, {44, 4, 3}, {56, 5, 1}, {60, 4, 2}};
static struct tag_swap_run const struct_game_globals_interface_tag_references_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 3}, {36, 5, 1}, {40, 4, 3}, {52, 5, 1}, {56, 4, 3}, {68, 5, 1}, {72, 4, 3}, {84, 5, 1}, {88, 4, 3}, {100, 5, 1}, {104, 4, 3}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 3}, {180, 5, 1}, {184, 4, 3}, {196, 5, 1}, {200, 4, 3}, {212, 5, 1}, {216, 4, 3}, {228, 5, 1}, {232, 4, 3}, {244, 5, 1}, {248, 4, 14}};
static struct tag_swap_run const struct_game_globals_multiplayer_information_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 3}, {36, 5, 2}, {44, 4, 1}, {48, 5, 1}, {52, 4, 3}, {64, 5, 1}, {68, 4, 3}, {80, 5, 1}, {84, 4, 3}, {96, 5, 2}};
static struct tag_swap_run const struct_game_globals_player_control_runs[] = {{0, 4, 27}, {108, 2, 2}, {112, 4, 2}, {120, 5, 2}};
static struct tag_swap_run const struct_game_globals_player_information_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 45}, {188, 5, 1}, {192, 4, 13}};
static struct tag_swap_run const struct_game_globals_rasterizer_data_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 3}, {36, 5, 1}, {40, 4, 3}, {52, 5, 1}, {56, 4, 3}, {68, 5, 1}, {72, 4, 3}, {84, 5, 1}, {88, 4, 3}, {100, 5, 1}, {104, 4, 18}, {176, 5, 1}, {180, 4, 3}, {192, 5, 1}, {196, 4, 3}, {208, 5, 1}, {212, 4, 3}, {224, 5, 1}, {228, 4, 3}, {240, 5, 1}, {244, 4, 3}, {256, 5, 1}, {260, 4, 3}, {272, 5, 1}, {276, 4, 3}, {288, 5, 1}, {292, 4, 3}, {304, 5, 1}, {308, 4, 15}, {368, 2, 2}, {372, 4, 11}, {416, 5, 1}, {420, 4, 2}};
static struct tag_swap_run const struct_game_globals_vehicle_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_gbxmodel_geometry_part_runs[] = {{0, 4, 1}, {4, 2, 1}, {8, 2, 2}, {12, 4, 6}, {36, 5, 2}, {44, 4, 1}, {48, 5, 2}, {56, 4, 1}, {60, 5, 2}, {68, 2, 2}, {72, 4, 3}, {84, 2, 2}, {88, 4, 4}, {130, 2, 1}};
static struct tag_swap_run const struct_glow_definition_runs[] = {{32, 2, 4}, {40, 4, 8}, {72, 2, 2}, {76, 4, 1}, {80, 2, 2}, {84, 4, 3}, {96, 2, 2}, {100, 4, 3}, {112, 2, 2}, {116, 4, 3}, {128, 2, 2}, {132, 4, 6}, {156, 2, 2}, {160, 4, 4}, {176, 2, 2}, {180, 4, 37}, {328, 5, 1}, {332, 4, 2}};
static struct tag_swap_run const struct_grenade_count_panel_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 6}, {40, 5, 1}, {44, 4, 6}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 1}, {92, 5, 2}, {100, 4, 1}, {104, 2, 2}, {108, 4, 2}, {116, 2, 2}, {120, 4, 9}, {156, 2, 2}, {160, 4, 2}, {168, 2, 2}, {176, 4, 3}, {188, 2, 2}};
static struct tag_swap_run const struct_grenade_hud_interface_definition_runs[] = {{0, 2, 2}, {4, 4, 8}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 6}, {76, 5, 1}, {80, 4, 6}, {104, 2, 2}, {108, 4, 2}, {116, 2, 4}, {124, 4, 1}, {128, 5, 2}, {136, 4, 1}, {140, 2, 2}, {144, 4, 2}, {152, 2, 2}, {156, 4, 6}, {180, 5, 1}, {184, 4, 6}, {208, 2, 2}, {212, 4, 2}, {220, 2, 4}, {228, 4, 1}, {232, 5, 2}, {240, 4, 1}, {244, 2, 2}, {248, 4, 2}, {256, 2, 2}, {260, 4, 9}, {296, 2, 2}, {300, 4, 2}, {308, 2, 2}, {316, 4, 3}, {328, 2, 2}, {332, 4, 1}, {336, 5, 1}, {340, 4, 3}, {352, 5, 2}, {360, 4, 1}, {364, 5, 2}, {372, 4, 18}, {444, 5, 1}, {448, 4, 14}};
static struct tag_swap_run const struct_hs_global_runs[] = {{32, 2, 2}, {36, 4, 2}};
static struct tag_swap_run const struct_hs_reference_runs[] = {{24, 4, 1}, {28, 5, 1}, {32, 4, 2}};
static struct tag_swap_run const struct_hs_script_runs[] = {{32, 2, 2}, {36, 4, 1}};
static struct tag_swap_run const struct_hs_source_file_runs[] = {{32, 4, 3}, {44, 5, 2}};
static struct tag_swap_run const struct_hud_absolute_placement_definition_runs[] = {{0, 2, 2}, {4, 4, 8}};
static struct tag_swap_run const struct_hud_globals_definition_runs[] = {{0, 2, 2}, {4, 4, 8}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 6}, {76, 5, 1}, {80, 4, 3}, {92, 5, 1}, {96, 4, 14}, {152, 5, 1}, {156, 4, 3}, {168, 5, 1}, {172, 4, 3}, {184, 5, 1}, {188, 4, 3}, {200, 5, 2}, {208, 4, 4}, {224, 2, 2}, {228, 4, 2}, {236, 2, 2}, {240, 4, 1}, {244, 5, 1}, {248, 4, 6}, {272, 2, 2}, {276, 4, 2}, {284, 2, 2}, {288, 4, 13}, {340, 5, 1}, {344, 4, 3}, {356, 5, 2}, {364, 4, 86}, {708, 5, 1}, {712, 4, 5}, {732, 2, 4}, {740, 4, 11}, {784, 2, 4}, {792, 4, 9}, {828, 5, 1}, {832, 4, 2}, {840, 2, 2}, {844, 4, 9}, {880, 2, 2}, {884, 4, 2}, {892, 2, 2}, {896, 4, 4}, {912, 2, 2}, {916, 4, 2}, {924, 2, 2}, {928, 4, 11}, {972, 5, 1}, {976, 4, 2}, {984, 2, 4}, {992, 4, 1}, {996, 5, 1}, {1000, 4, 26}};
static struct tag_swap_run const struct_hud_message_text_definition_runs[] = {{0, 4, 3}, {12, 5, 2}, {20, 4, 1}, {24, 5, 2}, {32, 4, 1}, {36, 5, 2}, {44, 4, 21}};
static struct tag_swap_run const struct_hud_number_definition_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}, {22, 2, 1}, {24, 4, 19}};
static struct tag_swap_run const struct_hud_screen_effect_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 5}, {28, 5, 1}, {32, 4, 3}, {44, 5, 1}, {48, 4, 4}, {64, 2, 2}, {68, 4, 10}, {108, 2, 2}, {112, 4, 7}, {140, 2, 2}, {144, 4, 10}};
static struct tag_swap_run const struct_hud_sound_definition_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 4}};
static struct tag_swap_run const struct_hud_state_message_definition_runs[] = {{32, 2, 2}, {40, 4, 6}};
static struct tag_swap_run const struct_hud_state_message_element_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_hud_waypoint_arrow_runs[] = {{32, 4, 5}, {52, 2, 4}, {60, 4, 11}};
static struct tag_swap_run const struct_icon_hud_element_definition_runs[] = {{0, 2, 4}, {8, 4, 1}, {14, 2, 1}};
static struct tag_swap_run const struct_item_collection_definition_runs[] = {{0, 4, 1}, {4, 5, 2}, {12, 2, 2}, {16, 4, 19}};
static struct tag_swap_run const struct_item_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 1}, {384, 2, 2}, {388, 4, 1}, {392, 2, 2}, {396, 4, 4}, {412, 2, 4}, {420, 4, 42}, {588, 5, 1}, {592, 4, 3}, {604, 5, 1}, {608, 4, 35}, {748, 5, 1}, {752, 4, 3}, {764, 5, 1}, {768, 4, 2}};
static struct tag_swap_run const struct_item_permutation_definition_runs[] = {{0, 4, 10}, {40, 5, 1}, {44, 4, 10}};
static struct tag_swap_run const struct_leaf_map_runs[] = {{0, 5, 1}, {4, 4, 1}, {8, 5, 2}, {16, 4, 1}, {20, 5, 2}};
static struct tag_swap_run const struct_leaf_portal_runs[] = {{0, 4, 4}, {16, 5, 2}};
static struct tag_swap_run const struct_lens_flare_definition_runs[] = {{0, 4, 5}, {20, 2, 2}, {24, 4, 3}, {36, 5, 1}, {40, 4, 2}, {48, 2, 2}, {128, 2, 2}, {132, 4, 1}, {160, 4, 2}, {196, 4, 1}, {200, 5, 2}};
static struct tag_swap_run const struct_lens_flare_reflection_runs[] = {{0, 2, 4}, {28, 4, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 2}, {60, 2, 2}, {64, 4, 12}, {112, 2, 2}, {116, 4, 2}};
static struct tag_swap_run const struct_light_definition_runs[] = {{0, 4, 11}, {52, 4, 9}, {100, 4, 1}, {104, 5, 1}, {108, 4, 2}, {116, 2, 2}, {120, 4, 2}, {128, 5, 1}, {132, 4, 2}, {140, 2, 2}, {144, 4, 1}, {148, 2, 2}, {152, 4, 1}, {156, 2, 2}, {160, 4, 1}, {172, 4, 1}, {176, 5, 1}, {180, 4, 2}, {244, 4, 1}, {248, 2, 2}};
static struct tag_swap_run const struct_light_fixture_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}, {40, 2, 2}, {44, 4, 7}};
static struct tag_swap_run const struct_light_volume_definition_runs[] = {{32, 2, 2}, {36, 4, 8}, {68, 2, 2}, {72, 4, 6}, {96, 5, 1}, {100, 4, 2}, {108, 2, 2}, {112, 4, 18}, {184, 2, 2}, {188, 4, 26}, {292, 5, 2}, {300, 4, 8}};
static struct tag_swap_run const struct_light_volume_frame_runs[] = {{0, 4, 44}};
static struct tag_swap_run const struct_lightning_definition_runs[] = {{0, 2, 2}, {20, 4, 2}, {44, 2, 4}, {52, 4, 1}, {56, 5, 1}, {60, 4, 2}, {152, 4, 1}, {156, 5, 2}, {164, 4, 1}, {168, 5, 2}};
static struct tag_swap_run const struct_lightning_marker_definition_runs[] = {{32, 2, 4}, {116, 4, 9}};
static struct tag_swap_run const struct_lightning_shader_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 8}, {76, 4, 1}, {80, 5, 1}, {84, 4, 2}, {92, 2, 4}, {100, 4, 3}, {112, 2, 2}, {116, 4, 3}, {128, 2, 2}, {132, 4, 7}};
static struct tag_swap_run const struct_looping_sound_definition_runs[] = {{0, 4, 12}, {48, 5, 1}, {52, 4, 3}, {64, 5, 2}, {72, 4, 1}, {76, 5, 2}};
static struct tag_swap_run const struct_looping_sound_detail_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 24}};
static struct tag_swap_run const struct_looping_sound_track_runs[] = {{0, 4, 13}, {52, 5, 1}, {56, 4, 3}, {68, 5, 1}, {72, 4, 3}, {84, 5, 1}, {88, 4, 11}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 2}};
static struct tag_swap_run const struct_machine_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 7}, {408, 2, 4}, {416, 4, 1}, {420, 5, 1}, {424, 4, 3}, {436, 5, 1}, {440, 4, 3}, {452, 5, 1}, {456, 4, 3}, {468, 5, 1}, {472, 4, 3}, {484, 5, 1}, {488, 4, 3}, {500, 5, 1}, {504, 4, 6}, {528, 5, 1}, {532, 4, 31}, {656, 2, 2}, {660, 4, 21}, {744, 2, 2}, {748, 4, 14}};
static struct tag_swap_run const struct_machine_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}, {40, 2, 2}, {44, 4, 1}};
static struct tag_swap_run const struct_map_leaf_runs[] = {{0, 4, 1}, {4, 5, 2}, {12, 4, 1}, {16, 5, 2}};
static struct tag_swap_run const struct_map_leaf_face_runs[] = {{0, 4, 2}, {8, 5, 2}};
static struct tag_swap_run const struct_mass_point_definition_runs[] = {{32, 2, 2}, {36, 4, 14}, {92, 2, 2}, {96, 4, 3}};
static struct tag_swap_run const struct_material_definition_runs[] = {{0, 4, 186}, {744, 5, 1}, {748, 4, 3}, {760, 5, 1}, {764, 4, 9}, {800, 5, 2}, {808, 4, 16}, {872, 5, 1}, {876, 4, 2}};
static struct tag_swap_run const struct_material_effect_runs[] = {{0, 4, 1}, {4, 5, 2}};
static struct tag_swap_run const struct_material_effect_material_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 2}};
static struct tag_swap_run const struct_material_effects_definition_runs[] = {{0, 4, 1}, {4, 5, 2}};
static struct tag_swap_run const struct_meter_hud_element_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 6}, {40, 5, 1}, {44, 4, 6}, {70, 2, 1}, {74, 2, 1}, {76, 4, 4}, {92, 5, 2}, {100, 4, 1}};
static struct tag_swap_run const struct_metered_panel_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 6}, {40, 5, 1}, {44, 4, 6}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 1}, {92, 5, 2}, {100, 4, 1}, {104, 2, 2}, {108, 4, 2}, {116, 2, 2}, {120, 4, 6}, {144, 5, 1}, {148, 4, 6}, {174, 2, 1}, {178, 2, 1}, {180, 4, 4}, {196, 5, 2}, {204, 4, 9}};
static struct tag_swap_run const struct_model_runs[] = {{0, 4, 7}, {28, 2, 6}, {40, 4, 34}, {176, 5, 2}, {184, 4, 1}, {188, 5, 2}, {196, 4, 1}, {200, 5, 2}, {208, 4, 1}, {212, 5, 2}, {220, 4, 1}, {224, 5, 2}};
static struct tag_swap_run const struct_model_geometry_runs[] = {{36, 4, 1}, {40, 5, 2}};
static struct tag_swap_run const struct_model_geometry_part_runs[] = {{0, 4, 1}, {4, 2, 1}, {8, 2, 2}, {12, 4, 6}, {36, 5, 2}, {44, 4, 1}, {48, 5, 2}, {56, 4, 1}, {60, 5, 2}, {68, 2, 2}, {72, 4, 1}, {76, 5, 2}, {84, 2, 2}, {88, 4, 2}, {96, 5, 2}};
static struct tag_swap_run const struct_model_marker_runs[] = {{32, 4, 6}, {56, 5, 2}};
static struct tag_swap_run const struct_model_marker_instance_runs[] = {{4, 4, 7}};
static struct tag_swap_run const struct_model_node_runs[] = {{32, 2, 4}, {40, 4, 29}};
static struct tag_swap_run const struct_model_region_runs[] = {{32, 4, 9}, {68, 5, 2}};
static struct tag_swap_run const struct_model_region_permutation_runs[] = {{32, 4, 1}, {36, 2, 2}, {40, 4, 6}, {64, 2, 6}, {76, 4, 1}, {80, 5, 2}};
static struct tag_swap_run const struct_model_region_permutation_marker_runs[] = {{32, 2, 2}, {36, 4, 7}};
static struct tag_swap_run const struct_model_shader_reference_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}, {16, 2, 2}, {20, 4, 3}};
static struct tag_swap_run const struct_model_triangle_element_runs[] = {{0, 2, 3}};
static struct tag_swap_run const struct_model_vertex_compressed_element_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_model_vertex_uncompressed_element_runs[] = {{0, 0, 0}};
static struct tag_swap_run const struct_motion_sensor_panel_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 6}, {40, 5, 1}, {44, 4, 6}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 1}, {92, 5, 2}, {100, 4, 1}, {104, 2, 2}, {108, 4, 2}, {116, 2, 2}, {120, 4, 6}, {144, 5, 1}, {148, 4, 6}, {172, 2, 2}, {176, 4, 2}, {184, 2, 4}, {192, 4, 1}, {196, 5, 2}, {204, 4, 9}};
static struct tag_swap_run const struct_move_position_definition_runs[] = {{0, 4, 7}, {28, 2, 1}, {32, 4, 2}, {40, 2, 2}, {44, 4, 9}};
static struct tag_swap_run const struct_multiplayer_scenario_description_runs[] = {{0, 4, 1}, {4, 5, 2}};
static struct tag_swap_run const struct_multiplayer_scenario_description_entry_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 2}, {64, 4, 1}};
static struct tag_swap_run const struct_multitexture_overlay_hud_element_definition_runs[] = {{0, 2, 4}, {8, 4, 8}, {40, 2, 6}, {52, 4, 13}, {104, 5, 1}, {108, 4, 3}, {120, 5, 1}, {124, 4, 3}, {136, 5, 1}, {140, 4, 2}, {148, 2, 4}, {156, 4, 47}, {344, 5, 2}, {352, 4, 32}};
static struct tag_swap_run const struct_multitexture_overlay_hud_element_effector_definition_runs[] = {{0, 4, 16}, {64, 2, 4}, {72, 4, 26}, {176, 2, 2}, {180, 4, 10}};
static struct tag_swap_run const struct_object_attachment_definition_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}, {48, 2, 4}, {56, 4, 4}};
static struct tag_swap_run const struct_object_change_color_definition_runs[] = {{0, 2, 2}, {4, 4, 8}, {36, 5, 2}};
static struct tag_swap_run const struct_object_change_color_permutation_runs[] = {{0, 4, 7}};
static struct tag_swap_run const struct_object_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}};
static struct tag_swap_run const struct_object_definition_widget_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 6}};
static struct tag_swap_run const struct_object_function_definition_runs[] = {{0, 4, 2}, {8, 2, 4}, {16, 4, 3}, {28, 2, 6}, {40, 4, 3}, {52, 2, 2}, {56, 4, 68}};
static struct tag_swap_run const struct_optional_meter_hud_element_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 6}, {40, 5, 1}, {44, 4, 6}, {70, 2, 1}, {74, 2, 1}, {76, 4, 4}, {92, 5, 2}, {100, 4, 1}};
static struct tag_swap_run const struct_optional_static_hud_element_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 6}, {40, 5, 1}, {44, 4, 6}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 1}, {92, 5, 2}, {100, 4, 1}};
static struct tag_swap_run const struct_particle_definition_runs[] = {{0, 4, 2}, {8, 5, 1}, {12, 4, 3}, {24, 5, 1}, {28, 4, 3}, {40, 5, 1}, {44, 4, 8}, {76, 5, 1}, {80, 4, 3}, {92, 5, 1}, {96, 4, 14}, {152, 2, 4}, {160, 4, 3}, {172, 2, 4}, {180, 4, 7}, {208, 2, 8}, {252, 4, 1}, {256, 5, 1}, {260, 4, 2}, {268, 2, 4}, {276, 4, 3}, {288, 2, 2}, {292, 4, 3}, {304, 2, 2}, {308, 4, 7}};
static struct tag_swap_run const struct_particle_system_definition_runs[] = {{56, 4, 1}, {60, 5, 1}, {64, 4, 2}, {72, 2, 2}, {76, 4, 2}, {84, 5, 2}, {92, 4, 1}, {96, 5, 2}};
static struct tag_swap_run const struct_particle_system_physics_constant_runs[] = {{0, 4, 1}};
static struct tag_swap_run const struct_particle_system_type_runs[] = {{32, 4, 1}, {36, 2, 4}, {44, 4, 1}, {84, 2, 2}, {88, 4, 2}, {96, 5, 2}, {104, 4, 1}, {108, 5, 2}, {116, 4, 1}, {120, 5, 2}};
static struct tag_swap_run const struct_particle_system_type_particle_state_runs[] = {{32, 4, 5}, {52, 5, 1}, {56, 4, 2}, {64, 2, 2}, {68, 4, 17}, {136, 5, 1}, {140, 4, 2}, {184, 2, 2}, {188, 4, 7}, {216, 2, 8}, {260, 4, 1}, {264, 5, 1}, {268, 4, 2}, {276, 2, 4}, {284, 4, 3}, {296, 2, 2}, {300, 4, 3}, {312, 2, 2}, {316, 4, 7}};
static struct tag_swap_run const struct_particle_system_type_state_runs[] = {{32, 4, 36}, {176, 2, 2}, {180, 4, 1}, {184, 5, 2}};
static struct tag_swap_run const struct_pathfinding_sphere_runs[] = {{0, 2, 2}, {4, 4, 7}};
static struct tag_swap_run const struct_physics_definition_runs[] = {{0, 4, 24}, {96, 5, 2}, {104, 4, 1}, {108, 5, 2}, {116, 4, 1}, {120, 5, 2}};
static struct tag_swap_run const struct_platoon_definition_runs[] = {{32, 4, 4}, {48, 2, 2}, {52, 4, 2}, {60, 2, 2}, {64, 4, 1}};
static struct tag_swap_run const struct_platoon_rule_runs[] = {{0, 2, 2}, {4, 4, 1}};
static struct tag_swap_run const struct_player_starting_location_runs[] = {{0, 4, 4}, {16, 2, 6}};
static struct tag_swap_run const struct_point_physics_definition_runs[] = {{0, 4, 16}};
static struct tag_swap_run const struct_powered_mass_point_definition_runs[] = {{32, 4, 24}};
static struct tag_swap_run const struct_powered_seat_definition_runs[] = {{0, 4, 17}};
static struct tag_swap_run const struct_predicted_resource_runs[] = {{0, 2, 2}, {4, 4, 1}};
static struct tag_swap_run const struct_projectile_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 1}, {384, 2, 6}, {396, 4, 1}, {400, 5, 1}, {404, 4, 7}, {432, 5, 1}, {436, 4, 15}, {496, 2, 2}, {500, 4, 1}, {504, 5, 1}, {508, 4, 3}, {520, 5, 1}, {524, 4, 3}, {536, 5, 1}, {540, 4, 3}, {552, 5, 1}, {556, 4, 6}, {580, 5, 2}};
static struct tag_swap_run const struct_projectile_material_response_definition_runs[] = {{0, 2, 2}, {4, 4, 1}, {8, 5, 1}, {12, 4, 2}, {36, 2, 2}, {40, 4, 6}, {64, 5, 1}, {68, 4, 2}, {92, 2, 2}, {96, 4, 3}, {108, 5, 1}, {112, 4, 2}, {144, 4, 4}};
static struct tag_swap_run const struct_recorded_animation_definition_runs[] = {{36, 2, 2}, {40, 4, 4}, {56, 5, 2}};
static struct tag_swap_run const struct_render_lighting_runs[] = {{0, 4, 3}, {12, 2, 2}, {16, 4, 12}, {64, 2, 2}, {68, 4, 12}};
static struct tag_swap_run const struct_scenario_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 3}, {36, 5, 1}, {40, 4, 3}, {52, 5, 2}, {60, 2, 2}, {64, 4, 1}, {68, 5, 2}, {76, 4, 41}, {240, 5, 2}, {248, 4, 1}, {252, 5, 2}, {260, 4, 3}, {272, 5, 2}, {280, 4, 1}, {284, 5, 2}, {292, 4, 57}, {520, 5, 2}, {528, 4, 1}, {532, 5, 2}, {540, 4, 1}, {544, 5, 2}, {552, 4, 1}, {556, 5, 2}, {564, 4, 1}, {568, 5, 2}, {576, 4, 1}, {580, 5, 2}, {588, 4, 1}, {592, 5, 2}, {600, 4, 1}, {604, 5, 2}, {612, 4, 1}, {616, 5, 2}, {624, 4, 1}, {628, 5, 2}, {636, 4, 1}, {640, 5, 2}, {648, 4, 1}, {652, 5, 2}, {660, 4, 1}, {664, 5, 2}, {672, 4, 1}, {676, 5, 2}, {684, 4, 1}, {688, 5, 2}, {696, 4, 1}, {700, 5, 2}, {708, 4, 1}, {712, 5, 2}, {720, 4, 1}, {724, 5, 2}, {732, 4, 1}, {736, 5, 2}, {744, 4, 1}, {748, 5, 2}, {756, 4, 1}, {760, 5, 2}, {768, 4, 1}, {772, 5, 2}, {780, 4, 1}, {784, 5, 2}, {792, 4, 1}, {796, 5, 2}, {804, 4, 1}, {808, 5, 2}, {816, 4, 1}, {820, 5, 2}, {828, 4, 1}, {832, 5, 2}, {840, 4, 1}, {844, 5, 2}, {852, 4, 1}, {856, 5, 2}, {864, 4, 1}, {868, 5, 2}, {876, 4, 1}, {880, 5, 2}, {888, 4, 1}, {892, 5, 2}, {900, 4, 1}, {904, 5, 2}, {912, 4, 1}, {916, 5, 2}, {924, 4, 1}, {928, 5, 2}, {936, 4, 1}, {940, 5, 2}, {948, 4, 1}, {952, 5, 2}, {960, 4, 1}, {964, 5, 2}, {972, 4, 22}, {1060, 5, 2}, {1068, 4, 1}, {1072, 5, 2}, {1080, 4, 1}, {1084, 5, 2}, {1092, 4, 1}, {1096, 5, 2}, {1104, 4, 1}, {1108, 5, 2}, {1116, 4, 1}, {1120, 5, 2}, {1128, 4, 1}, {1132, 5, 2}, {1140, 4, 3}, {1152, 5, 2}, {1160, 4, 3}, {1172, 5, 2}, {1180, 4, 1}, {1184, 5, 2}, {1192, 4, 1}, {1196, 5, 2}, {1204, 4, 1}, {1208, 5, 2}, {1216, 4, 1}, {1220, 5, 2}, {1228, 4, 7}, {1256, 5, 2}, {1264, 4, 1}, {1268, 5, 2}, {1276, 4, 1}, {1280, 5, 2}, {1288, 4, 28}, {1400, 5, 1}, {1404, 4, 3}, {1416, 5, 1}, {1420, 4, 3}, {1432, 5, 1}, {1436, 4, 3}, {1448, 5, 2}};
static struct tag_swap_run const struct_scenario_bsp_switch_trigger_volume_runs[] = {{0, 2, 4}};
static struct tag_swap_run const struct_scenario_conversation_line_runs[] = {{0, 2, 4}, {8, 4, 2}, {28, 4, 1}, {32, 5, 1}, {36, 4, 3}, {48, 5, 1}, {52, 4, 3}, {64, 5, 1}, {68, 4, 3}, {80, 5, 1}, {84, 4, 3}, {96, 5, 1}, {100, 4, 3}, {112, 5, 1}, {116, 4, 2}};
static struct tag_swap_run const struct_scenario_conversation_participant_runs[] = {{0, 2, 6}, {24, 2, 6}, {68, 4, 1}};
static struct tag_swap_run const struct_scenario_cutscene_camera_point_runs[] = {{0, 4, 1}, {36, 4, 17}};
static struct tag_swap_run const struct_scenario_cutscene_flag_runs[] = {{0, 4, 1}, {36, 4, 5}};
static struct tag_swap_run const struct_scenario_cutscene_title_runs[] = {{0, 4, 1}, {36, 4, 1}, {40, 2, 8}, {56, 4, 6}};
static struct tag_swap_run const struct_scenario_decal_palette_entry_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_scenario_detail_object_collection_palette_entry_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_scenario_device_group_runs[] = {{32, 4, 5}};
static struct tag_swap_run const struct_scenario_netgame_equipment_runs[] = {{0, 4, 1}, {4, 2, 6}, {16, 4, 17}, {84, 5, 1}, {88, 4, 14}};
static struct tag_swap_run const struct_scenario_netgame_flag_runs[] = {{0, 4, 4}, {16, 2, 2}, {20, 4, 32}};
static struct tag_swap_run const struct_scenario_object_name_runs[] = {{32, 2, 2}};
static struct tag_swap_run const struct_scenario_object_palette_entry_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 10}};
static struct tag_swap_run const struct_scenario_starting_equipment_runs[] = {{0, 4, 1}, {4, 2, 4}, {12, 4, 13}, {64, 5, 1}, {68, 4, 3}, {80, 5, 1}, {84, 4, 3}, {96, 5, 1}, {100, 4, 3}, {112, 5, 1}, {116, 4, 3}, {128, 5, 1}, {132, 4, 3}, {144, 5, 1}, {148, 4, 14}};
static struct tag_swap_run const struct_scenario_starting_profile_runs[] = {{32, 4, 3}, {44, 5, 1}, {48, 4, 2}, {56, 2, 2}, {60, 4, 1}, {64, 5, 1}, {68, 4, 2}, {76, 2, 2}};
static struct tag_swap_run const struct_scenario_structure_bsp_reference_runs[] = {{0, 4, 2}, {8, 5, 1}, {16, 4, 1}, {20, 5, 1}, {24, 4, 2}};
static struct tag_swap_run const struct_scenario_tag_reference_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_scenario_trigger_volume_runs[] = {{0, 2, 2}, {48, 4, 12}};
static struct tag_swap_run const struct_scenario_weapon_datum_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 5}, {64, 4, 2}, {72, 2, 4}, {80, 4, 3}};
static struct tag_swap_run const struct_scenery_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 5}, {64, 4, 2}};
static struct tag_swap_run const struct_shader_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}};
static struct tag_swap_run const struct_shader_effect_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 8}, {76, 4, 1}, {80, 5, 1}, {84, 4, 2}, {92, 2, 4}, {100, 4, 3}, {112, 2, 2}, {116, 4, 3}, {128, 2, 2}, {132, 4, 7}};
static struct tag_swap_run const struct_shader_environment_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {44, 4, 2}, {52, 5, 1}, {56, 4, 2}, {108, 2, 2}, {136, 4, 1}, {140, 5, 1}, {144, 4, 2}, {176, 2, 2}, {180, 4, 2}, {188, 5, 1}, {192, 4, 4}, {208, 5, 1}, {212, 4, 2}, {244, 2, 2}, {248, 4, 2}, {256, 5, 1}, {260, 4, 5}, {292, 4, 2}, {300, 5, 1}, {304, 4, 4}, {336, 2, 2}, {340, 4, 2}, {348, 2, 2}, {352, 4, 2}, {384, 2, 2}, {412, 4, 6}, {436, 2, 2}, {440, 4, 2}, {472, 4, 6}, {496, 2, 2}, {500, 4, 2}, {532, 4, 6}, {556, 2, 2}, {560, 4, 2}, {592, 4, 2}, {600, 5, 1}, {604, 4, 2}, {636, 2, 2}, {640, 4, 20}, {720, 2, 2}, {724, 4, 21}, {808, 5, 1}, {812, 4, 2}};
static struct tag_swap_run const struct_shader_layer_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_shader_model_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {56, 4, 1}, {76, 2, 1}, {108, 2, 4}, {116, 4, 7}, {156, 4, 3}, {168, 5, 1}, {172, 4, 2}, {188, 4, 1}, {192, 5, 1}, {196, 4, 2}, {212, 2, 2}, {216, 4, 2}, {224, 5, 1}, {228, 4, 3}, {252, 2, 2}, {256, 4, 3}, {268, 2, 2}, {272, 4, 3}, {284, 2, 2}, {288, 4, 5}, {316, 4, 11}, {360, 5, 1}, {364, 4, 2}};
static struct tag_swap_run const struct_shader_texture_animation_runs[] = {{0, 2, 2}, {4, 4, 3}, {16, 2, 2}, {20, 4, 3}, {32, 2, 2}, {36, 4, 5}};
static struct tag_swap_run const struct_shader_transparent_runs[] = {{2, 2, 5}, {12, 4, 2}, {20, 5, 1}, {24, 4, 3}, {36, 5, 2}, {44, 4, 1}, {48, 5, 2}};
static struct tag_swap_run const struct_shader_transparent_chicago_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}, {42, 2, 5}, {52, 4, 2}, {60, 5, 1}, {64, 4, 3}, {76, 5, 2}, {84, 4, 1}, {88, 5, 2}, {96, 4, 1}};
static struct tag_swap_run const struct_shader_transparent_chicago_extended_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}, {42, 2, 5}, {52, 4, 2}, {60, 5, 1}, {64, 4, 3}, {76, 5, 2}, {84, 4, 1}, {88, 5, 2}, {96, 4, 1}, {100, 5, 2}, {108, 4, 1}};
static struct tag_swap_run const struct_shader_transparent_chicago_map_runs[] = {{0, 2, 1}, {44, 2, 2}, {84, 4, 7}, {112, 5, 1}, {116, 4, 2}, {164, 2, 2}, {168, 4, 3}, {180, 2, 2}, {184, 4, 3}, {196, 2, 2}, {200, 4, 5}};
static struct tag_swap_run const struct_shader_transparent_generic_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}, {42, 2, 5}, {52, 4, 2}, {60, 5, 1}, {64, 4, 3}, {76, 5, 2}, {84, 4, 1}, {88, 5, 2}, {96, 4, 1}, {100, 5, 2}};
static struct tag_swap_run const struct_shader_transparent_generic_map_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 5, 1}, {36, 4, 2}, {44, 2, 2}, {48, 4, 3}, {60, 2, 2}, {64, 4, 3}, {76, 2, 2}, {80, 4, 5}};
static struct tag_swap_run const struct_shader_transparent_generic_stage_runs[] = {{0, 2, 4}, {8, 4, 13}, {60, 2, 26}};
static struct tag_swap_run const struct_shader_transparent_glass_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {84, 4, 5}, {104, 5, 1}, {108, 4, 2}, {136, 2, 2}, {140, 4, 9}, {176, 5, 1}, {180, 4, 4}, {196, 5, 1}, {200, 4, 2}, {336, 2, 2}, {340, 4, 2}, {348, 5, 1}, {352, 4, 4}, {368, 5, 1}, {372, 4, 2}};
static struct tag_swap_run const struct_shader_transparent_meter_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {76, 4, 1}, {80, 5, 1}, {84, 4, 2}};
static struct tag_swap_run const struct_shader_transparent_plasma_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 4}, {44, 2, 2}, {48, 4, 1}, {52, 2, 2}, {56, 4, 2}, {96, 4, 8}, {128, 2, 1}, {192, 4, 6}, {216, 5, 1}, {220, 4, 2}, {264, 4, 6}, {288, 5, 1}, {292, 4, 2}};
static struct tag_swap_run const struct_shader_transparent_water_definition_runs[] = {{0, 2, 2}, {4, 4, 7}, {32, 2, 6}, {76, 4, 1}, {80, 5, 1}, {84, 4, 2}, {108, 4, 8}, {156, 4, 1}, {160, 5, 1}, {164, 4, 2}, {188, 4, 4}, {204, 5, 1}, {208, 4, 2}, {216, 2, 2}, {220, 4, 2}, {292, 4, 1}, {296, 5, 2}};
static struct tag_swap_run const struct_sky_animation_runs[] = {{0, 2, 2}, {4, 4, 1}};
static struct tag_swap_run const struct_sky_definition_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 2}, {152, 4, 1}, {156, 5, 1}, {160, 4, 4}, {176, 5, 2}, {184, 4, 1}, {188, 5, 2}, {196, 4, 1}, {200, 5, 2}};
static struct tag_swap_run const struct_sky_light_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_sky_shader_function_runs[] = {{0, 4, 1}};
static struct tag_swap_run const struct_sound_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 10}, {60, 4, 3}, {84, 4, 3}, {108, 2, 2}, {112, 4, 1}, {116, 5, 1}, {120, 4, 2}, {128, 2, 2}, {132, 4, 6}, {156, 5, 2}};
static struct tag_swap_run const struct_sound_environment_definition_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 12}};
static struct tag_swap_run const struct_sound_permutation_runs[] = {{32, 4, 2}, {40, 2, 2}, {44, 4, 8}, {76, 5, 2}, {84, 4, 3}, {96, 5, 2}, {104, 4, 3}, {116, 5, 2}};
static struct tag_swap_run const struct_sound_pitch_range_runs[] = {{32, 4, 3}, {44, 2, 2}, {48, 4, 2}, {56, 2, 2}, {60, 4, 1}, {64, 5, 2}};
static struct tag_swap_run const struct_sound_scenery_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 1}};
static struct tag_swap_run const struct_squad_definition_runs[] = {{32, 2, 4}, {40, 4, 1}, {44, 2, 2}, {48, 4, 7}, {76, 2, 2}, {80, 4, 11}, {124, 2, 8}, {140, 4, 15}, {200, 5, 2}, {208, 4, 1}, {212, 5, 2}, {220, 4, 1}, {224, 5, 2}};
static struct tag_swap_run const struct_static_hud_element_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 6}, {40, 5, 1}, {44, 4, 6}, {68, 2, 2}, {72, 4, 2}, {80, 2, 4}, {88, 4, 1}, {92, 5, 2}, {100, 4, 1}};
static struct tag_swap_run const struct_string_list_runs[] = {{0, 4, 1}, {4, 5, 2}};
static struct tag_swap_run const struct_string_list_entry_runs[] = {{0, 4, 3}, {12, 5, 2}};
static struct tag_swap_run const struct_structure_background_sound_palette_entry_runs[] = {{32, 4, 1}, {36, 5, 1}, {40, 4, 2}};
static struct tag_swap_run const struct_structure_breakable_surface_runs[] = {{0, 4, 12}};
static struct tag_swap_run const struct_structure_bsp_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 12}, {56, 2, 2}, {60, 4, 12}, {108, 2, 2}, {112, 4, 14}, {168, 5, 2}, {176, 4, 1}, {180, 5, 2}, {188, 4, 1}, {192, 5, 2}, {200, 4, 7}, {228, 5, 2}, {236, 4, 1}, {240, 5, 2}, {248, 4, 1}, {252, 5, 2}, {260, 4, 1}, {264, 5, 2}, {272, 4, 4}, {288, 5, 2}, {296, 4, 1}, {300, 5, 2}, {308, 4, 1}, {312, 5, 2}, {320, 4, 3}, {332, 5, 2}, {340, 4, 1}, {344, 5, 2}, {352, 4, 4}, {368, 5, 2}, {376, 4, 1}, {380, 5, 2}, {388, 4, 1}, {392, 5, 2}, {400, 4, 1}, {404, 5, 2}, {412, 4, 7}, {440, 5, 2}, {448, 4, 1}, {452, 5, 2}, {460, 4, 7}, {488, 5, 2}, {496, 4, 1}, {500, 5, 2}, {508, 4, 1}, {512, 5, 2}, {520, 4, 1}, {524, 5, 2}, {532, 4, 3}, {544, 5, 2}, {552, 4, 7}, {580, 5, 2}, {588, 4, 1}, {592, 5, 2}, {600, 4, 1}, {604, 5, 2}, {612, 4, 2}, {620, 5, 1}, {624, 4, 1}, {628, 5, 2}, {636, 4, 1}, {640, 5, 2}};
static struct tag_swap_run const struct_structure_cluster_portal_runs[] = {{0, 2, 2}, {4, 4, 13}, {56, 5, 2}};
static struct tag_swap_run const struct_structure_cluster_schema_runs[] = {{0, 2, 8}, {16, 4, 7}, {44, 5, 2}, {52, 4, 1}, {56, 5, 2}, {64, 2, 2}, {68, 4, 1}, {72, 5, 2}, {80, 4, 1}, {84, 5, 2}, {92, 4, 1}, {96, 5, 2}};
static struct tag_swap_run const struct_structure_collision_material_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}, {16, 2, 2}};
static struct tag_swap_run const struct_structure_detail_object_data_runs[] = {{0, 4, 1}, {4, 5, 2}, {12, 4, 1}, {16, 5, 2}, {24, 4, 1}, {28, 5, 2}, {36, 4, 1}, {40, 5, 2}, {52, 4, 3}};
static struct tag_swap_run const struct_structure_fog_palette_entry_runs[] = {{32, 4, 1}, {36, 5, 1}, {40, 4, 2}};
static struct tag_swap_run const struct_structure_fog_plane_schema_runs[] = {{0, 2, 2}, {4, 4, 5}, {24, 5, 2}};
static struct tag_swap_run const struct_structure_fog_region_runs[] = {{36, 2, 2}};
static struct tag_swap_run const struct_structure_leaf_runs[] = {{6, 2, 3}, {12, 4, 1}};
static struct tag_swap_run const struct_structure_lens_flare_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_structure_lens_flare_marker_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_structure_lightmap_runs[] = {{0, 2, 2}, {4, 4, 5}, {24, 5, 2}};
static struct tag_swap_run const struct_structure_material_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}, {16, 2, 2}, {20, 4, 8}, {52, 2, 2}, {56, 4, 12}, {104, 2, 2}, {108, 4, 16}, {172, 2, 4}, {180, 4, 2}, {188, 5, 2}, {196, 2, 2}, {200, 4, 2}, {208, 5, 2}, {216, 4, 3}, {228, 5, 2}, {236, 4, 3}, {248, 5, 2}};
static struct tag_swap_run const struct_structure_mirror_runs[] = {{0, 4, 10}, {40, 5, 1}, {44, 4, 3}, {56, 5, 2}};
static struct tag_swap_run const struct_structure_portal_index_runs[] = {{0, 2, 1}};
static struct tag_swap_run const struct_structure_runtime_decal_runs[] = {{0, 4, 3}};
static struct tag_swap_run const struct_structure_sound_environment_palette_entry_runs[] = {{32, 4, 1}, {36, 5, 1}, {40, 4, 2}};
static struct tag_swap_run const struct_structure_subcluster_runs[] = {{0, 4, 7}, {28, 5, 2}};
static struct tag_swap_run const struct_structure_surface_runs[] = {{0, 2, 3}};
static struct tag_swap_run const struct_structure_surface_reference_runs[] = {{0, 4, 2}};
static struct tag_swap_run const struct_structure_weather_palette_entry_runs[] = {{32, 4, 1}, {36, 5, 1}, {40, 4, 2}, {48, 2, 2}, {84, 4, 12}, {132, 5, 1}, {136, 4, 6}, {160, 2, 2}, {196, 4, 11}};
static struct tag_swap_run const struct_structure_weather_polyhedron_runs[] = {{0, 4, 6}, {24, 5, 2}};
static struct tag_swap_run const struct_tag_reference_element_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_ui_widget_child_reference_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}, {48, 4, 1}, {52, 2, 3}};
static struct tag_swap_run const struct_ui_widget_collection_runs[] = {{0, 4, 1}, {4, 5, 2}};
static struct tag_swap_run const struct_ui_widget_collection_widget_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}};
static struct tag_swap_run const struct_ui_widget_conditional_reference_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}, {48, 4, 1}, {52, 2, 1}};
static struct tag_swap_run const struct_ui_widget_definition_runs[] = {{0, 2, 2}, {36, 2, 4}, {44, 4, 4}, {60, 5, 1}, {64, 4, 3}, {76, 5, 2}, {84, 4, 1}, {88, 5, 2}, {96, 4, 1}, {100, 5, 2}, {236, 4, 1}, {240, 5, 1}, {244, 4, 3}, {256, 5, 1}, {260, 4, 6}, {284, 2, 2}, {302, 2, 3}, {336, 4, 2}, {344, 5, 1}, {348, 4, 3}, {360, 5, 1}, {364, 4, 2}, {372, 2, 8}, {420, 4, 1}, {424, 5, 1}, {428, 4, 2}, {724, 4, 1}, {728, 5, 2}, {992, 4, 1}, {996, 5, 2}};
static struct tag_swap_run const struct_ui_widget_event_handler_reference_runs[] = {{0, 4, 1}, {4, 2, 2}, {8, 4, 1}, {12, 5, 1}, {16, 4, 3}, {28, 5, 1}, {32, 4, 2}};
static struct tag_swap_run const struct_ui_widget_game_data_input_reference_runs[] = {{0, 2, 1}};
static struct tag_swap_run const struct_ui_widget_search_and_replace_reference_runs[] = {{32, 2, 1}};
static struct tag_swap_run const struct_unit_camera_runs[] = {{64, 4, 4}, {80, 5, 2}};
static struct tag_swap_run const struct_unit_camera_track_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 5}};
static struct tag_swap_run const struct_unit_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 1}, {384, 2, 2}, {388, 4, 2}, {396, 5, 1}, {400, 4, 2}, {408, 2, 4}, {416, 4, 2}, {488, 4, 4}, {504, 5, 2}, {512, 4, 20}, {592, 5, 1}, {596, 4, 2}, {604, 2, 2}, {608, 4, 11}, {652, 5, 1}, {656, 4, 2}, {664, 2, 2}, {668, 4, 4}, {684, 5, 2}, {692, 4, 1}, {696, 5, 2}, {704, 4, 1}, {708, 2, 4}, {716, 4, 1}, {720, 5, 2}, {728, 4, 1}, {732, 5, 2}, {740, 4, 1}, {744, 5, 2}};
static struct tag_swap_run const struct_unit_dialogue_variant_definition_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 5, 1}, {16, 4, 2}};
static struct tag_swap_run const struct_unit_hud_interface_definition_runs[] = {{0, 2, 2}, {4, 4, 8}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 6}, {76, 5, 1}, {80, 4, 6}, {104, 2, 2}, {108, 4, 2}, {116, 2, 4}, {124, 4, 1}, {128, 5, 2}, {136, 4, 1}, {140, 2, 2}, {144, 4, 2}, {152, 2, 2}, {156, 4, 6}, {180, 5, 1}, {184, 4, 6}, {208, 2, 2}, {212, 4, 2}, {220, 2, 4}, {228, 4, 1}, {232, 5, 2}, {240, 4, 1}, {244, 2, 2}, {248, 4, 2}, {256, 2, 2}, {260, 4, 6}, {284, 5, 1}, {288, 4, 6}, {314, 2, 1}, {318, 2, 1}, {320, 4, 4}, {336, 5, 2}, {344, 4, 9}, {380, 2, 2}, {384, 4, 2}, {392, 2, 2}, {396, 4, 6}, {420, 5, 1}, {424, 4, 6}, {448, 2, 2}, {452, 4, 2}, {460, 2, 4}, {468, 4, 1}, {472, 5, 2}, {480, 4, 1}, {484, 2, 2}, {488, 4, 2}, {496, 2, 2}, {500, 4, 6}, {524, 5, 1}, {528, 4, 6}, {554, 2, 1}, {558, 2, 1}, {560, 4, 4}, {576, 5, 2}, {584, 4, 9}, {620, 2, 2}, {624, 4, 2}, {632, 2, 2}, {636, 4, 6}, {660, 5, 1}, {664, 4, 6}, {688, 2, 2}, {692, 4, 2}, {700, 2, 4}, {708, 4, 1}, {712, 5, 2}, {720, 4, 1}, {724, 2, 2}, {728, 4, 2}, {736, 2, 2}, {740, 4, 6}, {764, 5, 1}, {768, 4, 6}, {792, 2, 2}, {796, 4, 2}, {804, 2, 4}, {812, 4, 1}, {816, 5, 2}, {824, 4, 9}, {860, 2, 2}, {864, 4, 2}, {872, 2, 2}, {876, 4, 5}, {896, 2, 2}, {900, 4, 9}, {936, 5, 2}, {944, 4, 5}, {964, 5, 2}, {972, 4, 1}, {976, 5, 2}, {984, 4, 101}};
static struct tag_swap_run const struct_unit_hud_reference_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 10}};
static struct tag_swap_run const struct_unit_initial_weapon_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 7}};
static struct tag_swap_run const struct_unit_placement_runs[] = {{0, 2, 4}, {8, 4, 6}, {32, 2, 2}, {36, 4, 5}, {64, 4, 4}};
static struct tag_swap_run const struct_unit_seat_runs[] = {{0, 4, 1}, {68, 4, 16}, {196, 4, 4}, {212, 5, 2}, {220, 4, 1}, {224, 5, 2}, {232, 4, 1}, {236, 2, 2}, {240, 4, 3}, {252, 5, 1}, {256, 4, 7}};
static struct tag_swap_run const struct_vehicle_animation_runs[] = {{0, 4, 2}, {8, 2, 2}, {12, 4, 2}, {20, 2, 2}, {24, 4, 18}, {96, 5, 2}, {104, 4, 1}, {108, 5, 2}};
static struct tag_swap_run const struct_vehicle_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 1}, {384, 2, 2}, {388, 4, 2}, {396, 5, 1}, {400, 4, 2}, {408, 2, 4}, {416, 4, 2}, {488, 4, 4}, {504, 5, 2}, {512, 4, 20}, {592, 5, 1}, {596, 4, 2}, {604, 2, 2}, {608, 4, 11}, {652, 5, 1}, {656, 4, 2}, {664, 2, 2}, {668, 4, 4}, {684, 5, 2}, {692, 4, 1}, {696, 5, 2}, {704, 4, 1}, {708, 2, 4}, {716, 4, 1}, {720, 5, 2}, {728, 4, 1}, {732, 5, 2}, {740, 4, 1}, {744, 5, 2}, {752, 4, 1}, {756, 2, 2}, {760, 4, 9}, {796, 2, 4}, {816, 4, 2}, {832, 4, 2}, {868, 4, 1}, {944, 4, 1}, {948, 5, 1}, {952, 4, 3}, {964, 5, 1}, {968, 4, 3}, {980, 5, 1}, {984, 4, 3}, {996, 5, 1}, {1000, 4, 2}};
static struct tag_swap_run const struct_vehicle_suspension_runs[] = {{0, 2, 2}, {4, 4, 2}};
static struct tag_swap_run const struct_vibrate_frequency_definition_runs[] = {{0, 4, 2}, {8, 2, 2}, {12, 4, 2}};
static struct tag_swap_run const struct_virtual_keyboard_definition_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 1}, {24, 4, 3}, {36, 5, 1}, {40, 4, 3}, {52, 5, 2}};
static struct tag_swap_run const struct_virtual_keyboard_key_runs[] = {{0, 2, 8}, {16, 4, 1}, {20, 5, 1}, {24, 4, 3}, {36, 5, 1}, {40, 4, 3}, {52, 5, 1}, {56, 4, 3}, {68, 5, 1}, {72, 4, 2}};
static struct tag_swap_run const struct_water_ripple_runs[] = {{4, 4, 1}, {40, 4, 4}, {56, 2, 2}};
static struct tag_swap_run const struct_weapon_ammunition_object_runs[] = {{0, 2, 6}, {12, 4, 1}, {16, 5, 1}, {20, 4, 2}};
static struct tag_swap_run const struct_weapon_definition_runs[] = {{0, 2, 2}, {4, 4, 10}, {44, 5, 1}, {48, 4, 3}, {60, 5, 1}, {64, 4, 13}, {116, 5, 1}, {120, 4, 3}, {132, 5, 1}, {136, 4, 3}, {148, 5, 1}, {152, 4, 3}, {164, 5, 1}, {168, 4, 24}, {264, 2, 4}, {272, 4, 11}, {316, 2, 2}, {320, 4, 1}, {324, 5, 2}, {332, 4, 1}, {336, 5, 2}, {344, 4, 1}, {348, 5, 2}, {356, 4, 1}, {360, 5, 2}, {368, 4, 1}, {372, 5, 2}, {380, 4, 1}, {384, 2, 2}, {388, 4, 1}, {392, 2, 2}, {396, 4, 4}, {412, 2, 4}, {420, 4, 42}, {588, 5, 1}, {592, 4, 3}, {604, 5, 1}, {608, 4, 35}, {748, 5, 1}, {752, 4, 3}, {764, 5, 1}, {768, 4, 3}, {812, 2, 6}, {824, 4, 2}, {832, 5, 1}, {836, 4, 13}, {888, 5, 1}, {892, 4, 3}, {904, 5, 1}, {908, 4, 3}, {920, 5, 1}, {924, 4, 3}, {936, 5, 1}, {940, 4, 5}, {960, 5, 1}, {964, 4, 5}, {984, 2, 2}, {988, 4, 8}, {1020, 2, 2}, {1024, 4, 9}, {1060, 5, 1}, {1064, 4, 3}, {1076, 5, 1}, {1080, 4, 10}, {1120, 5, 1}, {1124, 4, 3}, {1136, 5, 1}, {1140, 4, 4}, {1156, 5, 1}, {1160, 4, 3}, {1172, 5, 1}, {1176, 4, 3}, {1188, 5, 1}, {1192, 4, 3}, {1204, 5, 1}, {1208, 4, 3}, {1220, 5, 2}, {1228, 4, 5}, {1248, 2, 2}, {1252, 4, 1}, {1256, 5, 2}, {1264, 4, 1}, {1268, 5, 2}, {1276, 4, 1}, {1280, 5, 2}};
static struct tag_swap_run const struct_weapon_firing_effect_runs[] = {{0, 2, 2}, {4, 4, 9}, {40, 5, 1}, {44, 4, 3}, {56, 5, 1}, {60, 4, 3}, {72, 5, 1}, {76, 4, 3}, {88, 5, 1}, {92, 4, 3}, {104, 5, 1}, {108, 4, 3}, {120, 5, 1}, {124, 4, 2}};
static struct tag_swap_run const struct_weapon_hud_crosshair_item_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 9}, {52, 2, 2}, {56, 4, 2}, {64, 2, 4}, {72, 4, 9}};
static struct tag_swap_run const struct_weapon_hud_crosshairs_element_runs[] = {{0, 2, 4}, {8, 4, 8}, {40, 5, 1}, {44, 4, 3}, {56, 5, 2}, {64, 4, 10}};
static struct tag_swap_run const struct_weapon_hud_element_header_runs[] = {{0, 2, 4}, {8, 4, 7}};
static struct tag_swap_run const struct_weapon_hud_interface_definition_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 2}, {16, 2, 6}, {28, 4, 8}, {60, 2, 2}, {64, 4, 9}, {100, 5, 2}, {108, 4, 1}, {112, 5, 2}, {120, 4, 1}, {124, 5, 2}, {132, 4, 1}, {136, 5, 2}, {144, 4, 1}, {148, 5, 2}, {156, 4, 2}, {164, 5, 2}, {172, 4, 1}, {176, 5, 2}, {184, 4, 33}, {316, 2, 4}, {324, 4, 1}, {330, 2, 1}, {332, 4, 12}};
static struct tag_swap_run const struct_weapon_hud_meter_element_runs[] = {{0, 2, 4}, {8, 4, 7}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 6}, {76, 5, 1}, {80, 4, 6}, {106, 2, 1}, {110, 2, 1}, {112, 4, 4}, {128, 5, 2}, {136, 4, 11}};
static struct tag_swap_run const struct_weapon_hud_number_element_runs[] = {{0, 2, 4}, {8, 4, 7}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 9}, {88, 2, 2}, {92, 4, 2}, {100, 2, 2}, {108, 4, 3}, {120, 2, 2}, {124, 4, 9}};
static struct tag_swap_run const struct_weapon_hud_overlay_definition_runs[] = {{0, 4, 1}, {4, 5, 1}, {8, 4, 3}, {20, 5, 2}};
static struct tag_swap_run const struct_weapon_hud_overlay_item_runs[] = {{0, 2, 2}, {4, 4, 2}, {12, 2, 2}, {16, 4, 9}, {52, 2, 2}, {56, 4, 2}, {64, 2, 6}, {76, 4, 15}};
static struct tag_swap_run const struct_weapon_hud_overlays_element_runs[] = {{0, 2, 4}, {8, 4, 8}, {40, 5, 1}, {44, 4, 3}, {56, 5, 2}, {64, 4, 10}};
static struct tag_swap_run const struct_weapon_hud_static_element_runs[] = {{0, 2, 4}, {8, 4, 7}, {36, 2, 2}, {40, 4, 2}, {48, 2, 2}, {52, 4, 6}, {76, 5, 1}, {80, 4, 6}, {104, 2, 2}, {108, 4, 2}, {116, 2, 4}, {124, 4, 1}, {128, 5, 2}, {136, 4, 11}};
static struct tag_swap_run const struct_weapon_magazine_definition_runs[] = {{0, 4, 1}, {4, 2, 8}, {20, 4, 1}, {24, 2, 2}, {28, 4, 3}, {40, 2, 8}, {56, 4, 1}, {60, 5, 1}, {64, 4, 3}, {76, 5, 1}, {80, 4, 6}, {104, 5, 2}};
static struct tag_swap_run const struct_weapon_trigger_definition_runs[] = {{0, 4, 8}, {32, 2, 8}, {48, 4, 8}, {80, 2, 2}, {84, 4, 3}, {96, 5, 1}, {100, 4, 2}, {108, 2, 2}, {112, 4, 10}, {152, 5, 1}, {156, 4, 28}, {268, 5, 2}};
static struct tag_swap_run const struct_weather_particle_system_definition_runs[] = {{0, 4, 10}, {40, 5, 2}};
static struct tag_swap_run const struct_weather_particle_type_definition_runs[] = {{32, 4, 36}, {176, 5, 1}, {180, 4, 57}, {408, 5, 1}, {412, 4, 2}, {420, 2, 4}, {428, 4, 7}, {456, 2, 8}, {500, 4, 1}, {504, 5, 1}, {508, 4, 2}, {516, 2, 4}, {524, 4, 3}, {536, 2, 2}, {540, 4, 3}, {552, 2, 2}, {556, 4, 7}};
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
	{"struct _biped_definition", 516, 10, struct__biped_definition_runs},
	{"struct _control_definition", 148, 9, struct__control_definition_runs},
	{"struct _device_definition", 276, 17, struct__device_definition_runs},
	{"struct _equipment_definition", 24, 4, struct__equipment_definition_runs},
	{"struct _item_definition", 396, 15, struct__item_definition_runs},
	{"struct _machine_definition", 148, 4, struct__machine_definition_runs},
	{"struct _object_definition", 380, 27, struct__object_definition_runs},
	{"struct _projectile_definition", 208, 18, struct__projectile_definition_runs},
	{"struct _unit_definition", 372, 29, struct__unit_definition_runs},
	{"struct _weapon_definition", 512, 44, struct__weapon_definition_runs},
	{"struct actor_definition", 1272, 19, struct_actor_definition_runs},
	{"struct actor_starting_location", 28, 3, struct_actor_starting_location_runs},
	{"struct actor_variant_change_colors", 32, 1, struct_actor_variant_change_colors_runs},
	{"struct actor_variant_definition", 568, 24, struct_actor_variant_definition_runs},
	{"struct ai_animation_reference_definition", 60, 3, struct_ai_animation_reference_definition_runs},
	{"struct ai_command_definition", 32, 4, struct_ai_command_definition_runs},
	{"struct ai_command_list_definition", 96, 10, struct_ai_command_list_definition_runs},
	{"struct ai_command_point_definition", 20, 1, struct_ai_command_point_definition_runs},
	{"struct ai_conversation", 116, 8, struct_ai_conversation_runs},
	{"struct ai_recording_reference_definition", 40, 1, struct_ai_recording_reference_definition_runs},
	{"struct ai_script_reference_definition", 40, 1, struct_ai_script_reference_definition_runs},
	{"struct animation", 180, 12, struct_animation_runs},
	{"struct animation_graph", 128, 22, struct_animation_graph_runs},
	{"struct animation_graph_animation_index", 2, 1, struct_animation_graph_animation_index_runs},
	{"struct animation_graph_device_animations", 96, 2, struct_animation_graph_device_animations_runs},
	{"struct animation_graph_ik_point", 64, 0, struct_animation_graph_ik_point_runs},
	{"struct animation_graph_node", 64, 2, struct_animation_graph_node_runs},
	{"struct animation_graph_object_overlay", 20, 2, struct_animation_graph_object_overlay_runs},
	{"struct animation_graph_sound_reference", 20, 3, struct_animation_graph_sound_reference_runs},
	{"struct animation_graph_unit_seat", 100, 10, struct_animation_graph_unit_seat_runs},
	{"struct animation_graph_weapon_animations", 28, 2, struct_animation_graph_weapon_animations_runs},
	{"struct animation_graph_weapon_class", 188, 10, struct_animation_graph_weapon_class_runs},
	{"struct animation_graph_weapon_type", 60, 2, struct_animation_graph_weapon_type_runs},
	{"struct antenna_definition", 208, 6, struct_antenna_definition_runs},
	{"struct antenna_vertex_definition", 128, 3, struct_antenna_vertex_definition_runs},
	{"struct auxilary_meter_definition", 324, 25, struct_auxilary_meter_definition_runs},
	{"struct auxilary_overlay_definition", 132, 14, struct_auxilary_overlay_definition_runs},
	{"struct biped_contact_point", 64, 0, struct_biped_contact_point_runs},
	{"struct biped_definition", 1268, 66, struct_biped_definition_runs},
	{"struct bitmap_data", 48, 4, struct_bitmap_data_runs},
	{"struct bitmap_group", 108, 13, struct_bitmap_group_runs},
	{"struct bitmap_group_sequence", 64, 3, struct_bitmap_group_sequence_runs},
	{"struct bitmap_group_sprite", 32, 2, struct_bitmap_group_sprite_runs},
	{"struct breakable_surface_particle_effect", 128, 3, struct_breakable_surface_particle_effect_runs},
	{"struct bsp2d_node", 20, 1, struct_bsp2d_node_runs},
	{"struct bsp2d_reference", 8, 1, struct_bsp2d_reference_runs},
	{"struct bsp3d_node", 12, 1, struct_bsp3d_node_runs},
	{"struct camera_track_control_point", 60, 1, struct_camera_track_control_point_runs},
	{"struct camera_track_definition", 48, 3, struct_camera_track_definition_runs},
	{"struct collision_bsp", 96, 16, struct_collision_bsp_runs},
	{"struct collision_edge", 24, 1, struct_collision_edge_runs},
	{"struct collision_leaf", 8, 2, struct_collision_leaf_runs},
	{"struct collision_model", 664, 32, struct_collision_model_runs},
	{"struct collision_node", 64, 5, struct_collision_node_runs},
	{"struct collision_surface", 12, 2, struct_collision_surface_runs},
	{"struct collision_vertex", 16, 1, struct_collision_vertex_runs},
	{"struct color_table_color", 48, 1, struct_color_table_color_runs},
	{"struct color_table_definition", 12, 2, struct_color_table_definition_runs},
	{"struct continuous_damage_effect_definition", 512, 5, struct_continuous_damage_effect_definition_runs},
	{"struct contrail_definition", 324, 14, struct_contrail_definition_runs},
	{"struct contrail_point_state", 104, 3, struct_contrail_point_state_runs},
	{"struct control_definition", 804, 53, struct_control_definition_runs},
	{"struct control_placement", 64, 7, struct_control_placement_runs},
	{"struct damage_definition", 220, 2, struct_damage_definition_runs},
	{"struct damage_effect_definition", 672, 19, struct_damage_effect_definition_runs},
	{"struct damage_permutation", 32, 0, struct_damage_permutation_runs},
	{"struct damage_region", 84, 4, struct_damage_region_runs},
	{"struct damage_resistance", 600, 28, struct_damage_resistance_runs},
	{"struct damage_resistance_material", 72, 3, struct_damage_resistance_material_runs},
	{"struct decal_definition", 268, 14, struct_decal_definition_runs},
	{"struct detail_object", 6, 1, struct_detail_object_runs},
	{"struct detail_object_cell_definition", 32, 2, struct_detail_object_cell_definition_runs},
	{"struct detail_object_collection_definition", 128, 6, struct_detail_object_collection_definition_runs},
	{"struct detail_object_type_definition", 96, 1, struct_detail_object_type_definition_runs},
	{"struct device_definition", 656, 44, struct_device_definition_runs},
	{"struct dialogue_definition", 4112, 514, struct_dialogue_definition_runs},
	{"struct effect_definition", 64, 6, struct_effect_definition_runs},
	{"struct effect_event_definition", 68, 4, struct_effect_event_definition_runs},
	{"struct effect_location_definition", 32, 0, struct_effect_location_definition_runs},
	{"struct effect_part_definition", 104, 4, struct_effect_part_definition_runs},
	{"struct effect_particles_definition", 232, 6, struct_effect_particles_definition_runs},
	{"struct encounter_definition", 176, 12, struct_encounter_definition_runs},
	{"struct encounter_player_starting_location", 52, 2, struct_encounter_player_starting_location_runs},
	{"struct equipment_definition", 800, 46, struct_equipment_definition_runs},
	{"struct equipment_placement", 40, 4, struct_equipment_placement_runs},
	{"struct firing_position_definition", 24, 3, struct_firing_position_definition_runs},
	{"struct flag_attachment_point", 52, 2, struct_flag_attachment_point_runs},
	{"struct flag_definition", 96, 10, struct_flag_definition_runs},
	{"struct fog_definition_full", 396, 17, struct_fog_definition_full_runs},
	{"struct font_character", 20, 2, struct_font_character_runs},
	{"struct font_character_index", 2, 1, struct_font_character_index_runs},
	{"struct font_character_table", 12, 2, struct_font_character_table_runs},
	{"struct font_header", 156, 16, struct_font_header_runs},
	{"struct game_globals", 428, 30, struct_game_globals_runs},
	{"struct game_globals_difficulty_information", 644, 1, struct_game_globals_difficulty_information_runs},
	{"struct game_globals_falling_damage", 152, 13, struct_game_globals_falling_damage_runs},
	{"struct game_globals_first_person_interface", 192, 17, struct_game_globals_first_person_interface_runs},
	{"struct game_globals_grenade", 68, 10, struct_game_globals_grenade_runs},
	{"struct game_globals_interface_tag_references", 304, 33, struct_game_globals_interface_tag_references_runs},
	{"struct game_globals_multiplayer_information", 160, 14, struct_game_globals_multiplayer_information_runs},
	{"struct game_globals_player_control", 128, 4, struct_game_globals_player_control_runs},
	{"struct game_globals_player_information", 244, 5, struct_game_globals_player_information_runs},
	{"struct game_globals_rasterizer_data", 428, 37, struct_game_globals_rasterizer_data_runs},
	{"struct game_globals_vehicle", 16, 3, struct_game_globals_vehicle_runs},
	{"struct gbxmodel_geometry_part", 132, 14, struct_gbxmodel_geometry_part_runs},
	{"struct glow_definition", 340, 18, struct_glow_definition_runs},
	{"struct grenade_count_panel_definition", 192, 21, struct_grenade_count_panel_definition_runs},
	{"struct grenade_hud_interface_definition", 504, 44, struct_grenade_hud_interface_definition_runs},
	{"struct hs_global", 92, 2, struct_hs_global_runs},
	{"struct hs_reference", 40, 3, struct_hs_reference_runs},
	{"struct hs_script", 92, 2, struct_hs_script_runs},
	{"struct hs_source_file", 52, 2, struct_hs_source_file_runs},
	{"struct hud_absolute_placement_definition", 36, 2, struct_hud_absolute_placement_definition_runs},
	{"struct hud_globals_definition", 1104, 56, struct_hud_globals_definition_runs},
	{"struct hud_message_text_definition", 128, 7, struct_hud_message_text_definition_runs},
	{"struct hud_number_definition", 100, 5, struct_hud_number_definition_runs},
	{"struct hud_screen_effect_definition", 184, 13, struct_hud_screen_effect_definition_runs},
	{"struct hud_sound_definition", 56, 3, struct_hud_sound_definition_runs},
	{"struct hud_state_message_definition", 64, 2, struct_hud_state_message_definition_runs},
	{"struct hud_state_message_element", 2, 0, struct_hud_state_message_element_runs},
	{"struct hud_waypoint_arrow", 104, 3, struct_hud_waypoint_arrow_runs},
	{"struct icon_hud_element_definition", 16, 3, struct_icon_hud_element_definition_runs},
	{"struct item_collection_definition", 92, 4, struct_item_collection_definition_runs},
	{"struct item_definition", 776, 42, struct_item_definition_runs},
	{"struct item_permutation_definition", 84, 3, struct_item_permutation_definition_runs},
	{"struct leaf_map", 28, 5, struct_leaf_map_runs},
	{"struct leaf_portal", 24, 2, struct_leaf_portal_runs},
	{"struct lens_flare_definition", 240, 11, struct_lens_flare_definition_runs},
	{"struct lens_flare_reflection", 128, 9, struct_lens_flare_reflection_runs},
	{"struct light_definition", 352, 20, struct_light_definition_runs},
	{"struct light_fixture_placement", 88, 6, struct_light_fixture_placement_runs},
	{"struct light_volume_definition", 332, 12, struct_light_volume_definition_runs},
	{"struct light_volume_frame", 176, 1, struct_light_volume_frame_runs},
	{"struct lightning_definition", 264, 10, struct_lightning_definition_runs},
	{"struct lightning_marker_definition", 228, 2, struct_lightning_marker_definition_runs},
	{"struct lightning_shader", 180, 12, struct_lightning_shader_runs},
	{"struct looping_sound_definition", 84, 6, struct_looping_sound_definition_runs},
	{"struct looping_sound_detail", 104, 3, struct_looping_sound_detail_runs},
	{"struct looping_sound_track", 160, 11, struct_looping_sound_track_runs},
	{"struct machine_definition", 804, 48, struct_machine_definition_runs},
	{"struct machine_placement", 64, 6, struct_machine_placement_runs},
	{"struct map_leaf", 24, 4, struct_map_leaf_runs},
	{"struct map_leaf_face", 16, 2, struct_map_leaf_face_runs},
	{"struct mass_point_definition", 128, 4, struct_mass_point_definition_runs},
	{"struct material_definition", 884, 9, struct_material_definition_runs},
	{"struct material_effect", 28, 2, struct_material_effect_runs},
	{"struct material_effect_material", 48, 5, struct_material_effect_material_runs},
	{"struct material_effects_definition", 12, 2, struct_material_effects_definition_runs},
	{"struct meter_hud_element_definition", 104, 11, struct_meter_hud_element_definition_runs},
	{"struct metered_panel_definition", 240, 23, struct_metered_panel_definition_runs},
	{"struct model", 232, 12, struct_model_runs},
	{"struct model_geometry", 48, 2, struct_model_geometry_runs},
	{"struct model_geometry_part", 104, 15, struct_model_geometry_part_runs},
	{"struct model_marker", 64, 2, struct_model_marker_runs},
	{"struct model_marker_instance", 32, 1, struct_model_marker_instance_runs},
	{"struct model_node", 156, 2, struct_model_node_runs},
	{"struct model_region", 76, 2, struct_model_region_runs},
	{"struct model_region_permutation", 88, 6, struct_model_region_permutation_runs},
	{"struct model_region_permutation_marker", 80, 2, struct_model_region_permutation_marker_runs},
	{"struct model_shader_reference", 32, 5, struct_model_shader_reference_runs},
	{"struct model_triangle_element", 6, 1, struct_model_triangle_element_runs},
	{"struct model_vertex_compressed_element", 32, 0, struct_model_vertex_compressed_element_runs},
	{"struct model_vertex_uncompressed_element", 68, 0, struct_model_vertex_uncompressed_element_runs},
	{"struct motion_sensor_panel_definition", 240, 24, struct_motion_sensor_panel_definition_runs},
	{"struct move_position_definition", 80, 5, struct_move_position_definition_runs},
	{"struct multiplayer_scenario_description", 12, 2, struct_multiplayer_scenario_description_runs},
	{"struct multiplayer_scenario_description_entry", 68, 6, struct_multiplayer_scenario_description_entry_runs},
	{"struct multitexture_overlay_hud_element_definition", 480, 14, struct_multitexture_overlay_hud_element_definition_runs},
	{"struct multitexture_overlay_hud_element_effector_definition", 220, 5, struct_multitexture_overlay_hud_element_effector_definition_runs},
	{"struct object_attachment_definition", 72, 5, struct_object_attachment_definition_runs},
	{"struct object_change_color_definition", 44, 3, struct_object_change_color_definition_runs},
	{"struct object_change_color_permutation", 28, 1, struct_object_change_color_permutation_runs},
	{"struct object_definition", 380, 27, struct_object_definition_runs},
	{"struct object_definition_widget", 32, 3, struct_object_definition_widget_runs},
	{"struct object_function_definition", 360, 7, struct_object_function_definition_runs},
	{"struct optional_meter_hud_element", 104, 11, struct_optional_meter_hud_element_runs},
	{"struct optional_static_hud_element", 104, 12, struct_optional_static_hud_element_runs},
	{"struct particle_definition", 356, 25, struct_particle_definition_runs},
	{"struct particle_system_definition", 104, 8, struct_particle_system_definition_runs},
	{"struct particle_system_physics_constant", 4, 1, struct_particle_system_physics_constant_runs},
	{"struct particle_system_type", 128, 10, struct_particle_system_type_runs},
	{"struct particle_system_type_particle_state", 376, 19, struct_particle_system_type_particle_state_runs},
	{"struct particle_system_type_state", 192, 4, struct_particle_system_type_state_runs},
	{"struct pathfinding_sphere", 32, 2, struct_pathfinding_sphere_runs},
	{"struct physics_definition", 128, 6, struct_physics_definition_runs},
	{"struct platoon_definition", 172, 5, struct_platoon_definition_runs},
	{"struct platoon_rule", 8, 2, struct_platoon_rule_runs},
	{"struct player_starting_location", 52, 2, struct_player_starting_location_runs},
	{"struct point_physics_definition", 64, 1, struct_point_physics_definition_runs},
	{"struct powered_mass_point_definition", 128, 1, struct_powered_mass_point_definition_runs},
	{"struct powered_seat_definition", 68, 1, struct_powered_seat_definition_runs},
	{"struct predicted_resource", 8, 2, struct_predicted_resource_runs},
	{"struct projectile_definition", 588, 45, struct_projectile_definition_runs},
	{"struct projectile_material_response_definition", 160, 13, struct_projectile_material_response_definition_runs},
	{"struct recorded_animation_definition", 64, 3, struct_recorded_animation_definition_runs},
	{"struct render_lighting", 116, 5, struct_render_lighting_runs},
	{"struct scenario", 1456, 135, struct_scenario_runs},
	{"struct scenario_bsp_switch_trigger_volume", 8, 1, struct_scenario_bsp_switch_trigger_volume_runs},
	{"struct scenario_conversation_line", 124, 15, struct_scenario_conversation_line_runs},
	{"struct scenario_conversation_participant", 84, 3, struct_scenario_conversation_participant_runs},
	{"struct scenario_cutscene_camera_point", 104, 2, struct_scenario_cutscene_camera_point_runs},
	{"struct scenario_cutscene_flag", 92, 2, struct_scenario_cutscene_flag_runs},
	{"struct scenario_cutscene_title", 96, 4, struct_scenario_cutscene_title_runs},
	{"struct scenario_decal_palette_entry", 16, 3, struct_scenario_decal_palette_entry_runs},
	{"struct scenario_detail_object_collection_palette_entry", 48, 3, struct_scenario_detail_object_collection_palette_entry_runs},
	{"struct scenario_device_group", 52, 1, struct_scenario_device_group_runs},
	{"struct scenario_netgame_equipment", 144, 5, struct_scenario_netgame_equipment_runs},
	{"struct scenario_netgame_flag", 148, 3, struct_scenario_netgame_flag_runs},
	{"struct scenario_object_name", 36, 1, struct_scenario_object_name_runs},
	{"struct scenario_object_palette_entry", 48, 3, struct_scenario_object_palette_entry_runs},
	{"struct scenario_starting_equipment", 204, 15, struct_scenario_starting_equipment_runs},
	{"struct scenario_starting_profile", 104, 8, struct_scenario_starting_profile_runs},
	{"struct scenario_structure_bsp_reference", 32, 5, struct_scenario_structure_bsp_reference_runs},
	{"struct scenario_tag_reference", 16, 3, struct_scenario_tag_reference_runs},
	{"struct scenario_trigger_volume", 96, 2, struct_scenario_trigger_volume_runs},
	{"struct scenario_weapon_datum", 92, 7, struct_scenario_weapon_datum_runs},
	{"struct scenery_placement", 72, 5, struct_scenery_placement_runs},
	{"struct shader", 40, 3, struct_shader_runs},
	{"struct shader_effect_definition", 180, 12, struct_shader_effect_definition_runs},
	{"struct shader_environment_definition", 836, 46, struct_shader_environment_definition_runs},
	{"struct shader_layer", 16, 3, struct_shader_layer_runs},
	{"struct shader_model_definition", 440, 26, struct_shader_model_definition_runs},
	{"struct shader_texture_animation", 56, 6, struct_shader_texture_animation_runs},
	{"struct shader_transparent", 56, 7, struct_shader_transparent_runs},
	{"struct shader_transparent_chicago_definition", 108, 11, struct_shader_transparent_chicago_definition_runs},
	{"struct shader_transparent_chicago_extended_definition", 120, 13, struct_shader_transparent_chicago_extended_definition_runs},
	{"struct shader_transparent_chicago_map", 220, 11, struct_shader_transparent_chicago_map_runs},
	{"struct shader_transparent_generic_definition", 108, 12, struct_shader_transparent_generic_definition_runs},
	{"struct shader_transparent_generic_map", 100, 10, struct_shader_transparent_generic_map_runs},
	{"struct shader_transparent_generic_stage", 112, 3, struct_shader_transparent_generic_stage_runs},
	{"struct shader_transparent_glass_definition", 480, 18, struct_shader_transparent_glass_definition_runs},
	{"struct shader_transparent_meter_definition", 260, 6, struct_shader_transparent_meter_definition_runs},
	{"struct shader_transparent_plasma_definition", 332, 15, struct_shader_transparent_plasma_definition_runs},
	{"struct shader_transparent_water_definition", 320, 17, struct_shader_transparent_water_definition_runs},
	{"struct sky_animation", 36, 2, struct_sky_animation_runs},
	{"struct sky_definition", 208, 13, struct_sky_definition_runs},
	{"struct sky_light", 116, 3, struct_sky_light_runs},
	{"struct sky_shader_function", 36, 1, struct_sky_shader_function_runs},
	{"struct sound_definition", 164, 12, struct_sound_definition_runs},
	{"struct sound_environment_definition", 72, 3, struct_sound_environment_definition_runs},
	{"struct sound_permutation", 124, 8, struct_sound_permutation_runs},
	{"struct sound_pitch_range", 72, 6, struct_sound_pitch_range_runs},
	{"struct sound_scenery_placement", 40, 4, struct_sound_scenery_placement_runs},
	{"struct squad_definition", 232, 13, struct_squad_definition_runs},
	{"struct static_hud_element_definition", 104, 12, struct_static_hud_element_definition_runs},
	{"struct string_list", 12, 2, struct_string_list_runs},
	{"struct string_list_entry", 20, 2, struct_string_list_entry_runs},
	{"struct structure_background_sound_palette_entry", 116, 3, struct_structure_background_sound_palette_entry_runs},
	{"struct structure_breakable_surface", 48, 1, struct_structure_breakable_surface_runs},
	{"struct structure_bsp", 648, 64, struct_structure_bsp_runs},
	{"struct structure_cluster_portal", 64, 3, struct_structure_cluster_portal_runs},
	{"struct structure_cluster_schema", 104, 12, struct_structure_cluster_schema_runs},
	{"struct structure_collision_material", 20, 4, struct_structure_collision_material_runs},
	{"struct structure_detail_object_data", 64, 9, struct_structure_detail_object_data_runs},
	{"struct structure_fog_palette_entry", 136, 3, struct_structure_fog_palette_entry_runs},
	{"struct structure_fog_plane_schema", 32, 3, struct_structure_fog_plane_schema_runs},
	{"struct structure_fog_region", 40, 1, struct_structure_fog_region_runs},
	{"struct structure_leaf", 16, 2, struct_structure_leaf_runs},
	{"struct structure_lens_flare", 16, 3, struct_structure_lens_flare_runs},
	{"struct structure_lens_flare_marker", 16, 1, struct_structure_lens_flare_marker_runs},
	{"struct structure_lightmap", 32, 3, struct_structure_lightmap_runs},
	{"struct structure_material", 256, 19, struct_structure_material_runs},
	{"struct structure_mirror", 64, 4, struct_structure_mirror_runs},
	{"struct structure_portal_index", 2, 1, struct_structure_portal_index_runs},
	{"struct structure_runtime_decal", 16, 1, struct_structure_runtime_decal_runs},
	{"struct structure_sound_environment_palette_entry", 80, 3, struct_structure_sound_environment_palette_entry_runs},
	{"struct structure_subcluster", 36, 2, struct_structure_subcluster_runs},
	{"struct structure_surface", 6, 1, struct_structure_surface_runs},
	{"struct structure_surface_reference", 8, 1, struct_structure_surface_reference_runs},
	{"struct structure_weather_palette_entry", 240, 9, struct_structure_weather_palette_entry_runs},
	{"struct structure_weather_polyhedron", 32, 2, struct_structure_weather_polyhedron_runs},
	{"struct tag_reference_element", 16, 3, struct_tag_reference_element_runs},
	{"struct ui_widget_child_reference", 80, 5, struct_ui_widget_child_reference_runs},
	{"struct ui_widget_collection", 12, 2, struct_ui_widget_collection_runs},
	{"struct ui_widget_collection_widget", 16, 3, struct_ui_widget_collection_widget_runs},
	{"struct ui_widget_conditional_reference", 80, 5, struct_ui_widget_conditional_reference_runs},
	{"struct ui_widget_definition", 1004, 30, struct_ui_widget_definition_runs},
	{"struct ui_widget_event_handler_reference", 72, 7, struct_ui_widget_event_handler_reference_runs},
	{"struct ui_widget_game_data_input_reference", 36, 1, struct_ui_widget_game_data_input_reference_runs},
	{"struct ui_widget_search_and_replace_reference", 34, 1, struct_ui_widget_search_and_replace_reference_runs},
	{"struct unit_camera", 88, 2, struct_unit_camera_runs},
	{"struct unit_camera_track", 28, 3, struct_unit_camera_track_runs},
	{"struct unit_definition", 752, 56, struct_unit_definition_runs},
	{"struct unit_dialogue_variant_definition", 24, 4, struct_unit_dialogue_variant_definition_runs},
	{"struct unit_hud_interface_definition", 1388, 96, struct_unit_hud_interface_definition_runs},
	{"struct unit_hud_reference", 48, 3, struct_unit_hud_reference_runs},
	{"struct unit_initial_weapon", 36, 3, struct_unit_initial_weapon_runs},
	{"struct unit_placement", 120, 5, struct_unit_placement_runs},
	{"struct unit_seat", 284, 11, struct_unit_seat_runs},
	{"struct vehicle_animation", 116, 8, struct_vehicle_animation_runs},
	{"struct vehicle_definition", 1008, 72, struct_vehicle_definition_runs},
	{"struct vehicle_suspension", 20, 2, struct_vehicle_suspension_runs},
	{"struct vibrate_frequency_definition", 20, 3, struct_vibrate_frequency_definition_runs},
	{"struct virtual_keyboard_definition", 60, 8, struct_virtual_keyboard_definition_runs},
	{"struct virtual_keyboard_key", 80, 10, struct_virtual_keyboard_key_runs},
	{"struct water_ripple", 76, 3, struct_water_ripple_runs},
	{"struct weapon_ammunition_object", 28, 4, struct_weapon_ammunition_object_runs},
	{"struct weapon_definition", 1288, 85, struct_weapon_definition_runs},
	{"struct weapon_firing_effect", 132, 14, struct_weapon_firing_effect_runs},
	{"struct weapon_hud_crosshair_item", 108, 8, struct_weapon_hud_crosshair_item_runs},
	{"struct weapon_hud_crosshairs_element", 104, 6, struct_weapon_hud_crosshairs_element_runs},
	{"struct weapon_hud_element_header", 36, 2, struct_weapon_hud_element_header_runs},
	{"struct weapon_hud_interface_definition", 380, 25, struct_weapon_hud_interface_definition_runs},
	{"struct weapon_hud_meter_element", 180, 13, struct_weapon_hud_meter_element_runs},
	{"struct weapon_hud_number_element", 160, 12, struct_weapon_hud_number_element_runs},
	{"struct weapon_hud_overlay_definition", 28, 4, struct_weapon_hud_overlay_definition_runs},
	{"struct weapon_hud_overlay_item", 136, 8, struct_weapon_hud_overlay_item_runs},
	{"struct weapon_hud_overlays_element", 104, 6, struct_weapon_hud_overlays_element_runs},
	{"struct weapon_hud_static_element", 180, 14, struct_weapon_hud_static_element_runs},
	{"struct weapon_magazine_definition", 112, 12, struct_weapon_magazine_definition_runs},
	{"struct weapon_trigger_definition", 276, 12, struct_weapon_trigger_definition_runs},
	{"struct weather_particle_system_definition", 48, 2, struct_weather_particle_system_definition_runs},
	{"struct weather_particle_type_definition", 604, 17, struct_weather_particle_type_definition_runs},
	{"struct wind_definition", 64, 1, struct_wind_definition_runs},
	{"word", 2, 1, word_runs},
};

long const tag_swap_type_count = 324;
