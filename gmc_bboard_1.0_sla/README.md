
Can do the following via the command line:
	(a) Move motor at speed forward/reverse (using motor's vref via pwm)
	(b) Move motor position reverse N-quadrature counts.
	(c) Move motor to home-switch and stop, display quadrature-position at home-switch.
	(d) Move motor across home-switch 3-times recording quadrature-position;
	    display x3 values recored.

================================================================================================
Build
================================================================================================

To build, do:
# cd gmc_bboard_1.0_sla
# make clean
# make

NOTE: The bbbiolib is used here and the version used is located in this dir
      under ./lib/BBBio_lib


================================================================================================
More Description
================================================================================================

There is help via the cmdline by doing:
	# gmc --help
including usage examples, repeated here:

 Operations::
 
   init => perform latch-init and put motor-ctrl regs to known state
 
   <motor-id>  <operation> <optional-args...>
       <motor-id>: is either: az|ti|el
                   and az=azimuth, ti=tilt, el=elevation
 
       <oper>: motor-operation is one of the following:
              en  => enable motor (mux-enable, brake-off)
              dis => disable motor (mux-disable, do not! apply brake)
              fcs => forward at specified duty-cycle (ex; 10.0=10.0%)
              rcs => reverse at specified duty-cycle (ex; 20.0=20.0%)
              mp  => move motor num quadrature-position counts (in reverse)
            ghome => move motor to home-switch and stop
           xghome => move motor pass home-switch, then back (two cycles)
 
   examples):
     To initialize [mux-init+mux_reset], do:
           # gmc init
 
     To set az forward speed at 30% (after gmc init), do:
           # gmc az fcs 30.0
           # gmz az en
 
     To now stop az motor, do:
           # gmc az dis
 
     To move motor a number of quadrature position counts (in reverse), do:
           # gmc az en    (if not already enabled)
           # gmc az mp 400
           => stopped at qpos: 400
 
     To move motor to home switch and stop (q-pos at home displayed), do:
           # gmc az en    (if not already enabled)
           # gmc az ghome
           => qpos at home: 3316
 
     To move across home-switch x3 times and stop (x3 q-pos displayed), do:
           # gmc az en    (if not already enabled)
           # gmc az xghome
           => qpos at home: [249, 204, -16]
 
================================================================================================
