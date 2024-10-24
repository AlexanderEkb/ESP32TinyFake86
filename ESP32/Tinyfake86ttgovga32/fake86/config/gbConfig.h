#ifndef _GB_CONFIG_H
  #define _GB_CONFIG_H


  //Section Optimice
  #define use_lib_fast_push
  #define use_lib_fast_pop
  #define use_lib_fast_op_add16
  #define use_lib_fast_op_sub16
  #define use_lib_fast_op_and8
  #define use_lib_fast_op_add8
  #define use_lib_fast_op_writew86
  #define use_lib_fast_op_xor8
  #define use_lib_fast_op_or16
  #define use_lib_fast_op_or8
  #define use_lib_fast_op_and16
  #define use_lib_fast_flag_log8
  #define use_lib_fast_flag_log16
  //#define use_lib_fast_flag_adc8
  //#define use_lib_fast_flag_adc16
  //#define use_lib_fast_flag_add8
  //#define use_lib_fast_flag_add16
  //#define use_lib_fast_flag_sbb8
  //#define use_lib_fast_flag_sbb16
  #define use_lib_fast_op_xor16
  //#define use_lib_fast_flag_sub8
  //#define use_lib_fast_flag_sub16
  #define use_lib_fast_op_sub8

  #define use_lib_fast_readw86
  #define use_lib_fast_flag_szp8
  #define use_lib_fast_flag_szp16

  #define use_lib_fast_op_adc8
  #define use_lib_fast_op_adc16
  #define use_lib_fast_op_sbb8
  #define use_lib_fast_op_sbb16

  #define use_lib_fast_modregrm
  #define use_lib_fast_readrm16
  #define use_lib_fast_readrm8
  #define use_lib_fast_writerm16
  #define use_lib_fast_writerm8
  #define use_lib_fast_op_div8


  #define use_lib_fast_decodeflagsword
  #define use_lib_fast_makeflagsword

  //#define use_lib_fast_doirq


  //#define use_lib_adlib
  //#define use_lib_disneysound
  //#define use_lib_mouse
  //#define use_lib_net

  //Usar 1 solo nucleo
  //  #define use_lib_singlecore

  //#define use_lib_snapshot
  //#define use_lib_limit_256KB

  #define gb_max_portram 0x3FF
  #define use_lib_limit_portram

  //#define use_lib_not_use_callback_port
  #define use_lib_fast_boot

  //#define use_lib_not_use_stretchblit
  //#define use_lib_not_use_roughblit
  //#define use_lib_not_use_doubleblit

  #define use_lib_force_sdl_blit
  #define use_lib_force_sdl_direct_vga
  #define use_lib_force_sdl_8bpp
  //milisegundos espera en cada frame
  #define use_lib_delay_tick_cpu_auto 0
  #define use_lib_delay_tick_cpu_milis 0
  #define use_lib_vga_poll_milis 10
  #define use_lib_keyboard_poll_milis 20
  #define use_lib_timers_poll_milis 54
  //Logs
  #define use_lib_log_serial

  #define DEFAULT_HDD_IMAGE "/sd/PC/HDDs/hdd0.img"
#endif
