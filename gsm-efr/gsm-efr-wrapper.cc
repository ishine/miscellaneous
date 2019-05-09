#include "gsm-efr-wrapper.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "c-code/basic_op.h"
#include "c-code/sig_proc.h"
#include "c-code/count.h"
#include "c-code/codec.h"
#include "c-code/cnst.h"
#include "c-code/n_stack.h"
#include "c-code/e_homing.h"
#include "c-code/d_homing.h"
#include "c-code/dtx.h"
#include "base/timer.h"
Word16 dtx_mode;
extern Word16 txdtx_ctrl;

#define WHOLE_FRAME 57
#define TO_FIRST_SUBFRAME 18
Word16 synth_buf[L_FRAME + M];

void GsmEfrWrapper::Encode(const char *pcm_in, int in_samples, short * efr_enc) {
  reset_enc (); /* Bring the encoder, VAD and DTX to the initial state */
  /* Loop for each "L_FRAME" speech data. */
  extern Word16 * new_speech;
  // int frame = 0;
  // int num_frames_ = in_samples / samples_per_frame_;
  int pos = 0;

  Word16 prm[PRM_SIZE];       /* Analysis parameters.                  */
  Word16 serial[SERIAL_SIZE - 1]; /* Output bitstream buffer               */
  Word16 syn[L_FRAME];        /* Buffer for synthesis speech           */

  const short * pcm_short = (const short*)pcm_in;
  for (int iframe = 0; iframe < num_frames_; iframe++) {
    for (int isample = 0 ; isample < L_FRAME; isample++) {
      new_speech[isample] =  pcm_short[iframe * L_FRAME + isample];
    }
    /* Check whether this frame is an encoder homing frame */
    // Word16 reset_flag = encoder_homing_frame_test (new_speech);

    for (int i = 0; i < L_FRAME; i++) { /* Delete the 3 LSBs (13-bit input) */
      new_speech[i] = new_speech[i] & 0xfff8;
      // logic16 ();
      // move16 ();
    }
    Pre_Process (new_speech, L_FRAME);           /* filter + downscaling */
    Coder_12k2 (prm, syn);  /* Find speech parameters   */
    // test (); logic16 ();
    if ((txdtx_ctrl & TX_SP_FLAG) == 0) {
      /* Write comfort noise parameters into the parameter frame.
      Use old parameters in case SID frame is not to be updated */
      CN_encoding (prm, txdtx_ctrl);
    }
    Prm2bits_12k2 (prm, &serial[0]); /* Parameters to serial bits */
    // test (); logic16 ();
    if ((txdtx_ctrl & TX_SP_FLAG) == 0) {
      /* Insert SID codeword into the serial parameter frame */
      sid_codeword_encoding (&serial[0]);
    }
    /* Write the bit stream to file */
    // fwrite (serial, sizeof (Word16), (SERIAL_SIZE - 1), f_serial);
    std::memcpy(efr_enc + pos, serial, sizeof(Word16) * (SERIAL_SIZE - 1));
    pos += (SERIAL_SIZE - 1);

    /* Write the VAD- and SP-flags to file after the speech
    parameter bit stream */
    Word16 vad = 0;
    Word16 sp = 0;

    if ((txdtx_ctrl & TX_VAD_FLAG) != 0) {vad = 1;}
    if ((txdtx_ctrl & TX_SP_FLAG) != 0) {sp = 1;}
    efr_enc[pos] = vad; pos += 1;
    efr_enc[pos] = sp; pos += 1;
    // if (reset_flag != 0) {
    //   reset_enc (); Bring the encoder, VAD and DTX to the home state
    // }
  }
}

static void random_parameters (Word16 serial_params[]) {
  static Word32 L_PN_seed = 0x321CEDE2L;
  Word16 i;

  /* Set the 244 speech parameter bits to random bit values */
  /* Function pseudonoise() is defined in dtx.c             */
  /*--------------------------------------------------------*/

  for (i = 0; i < 244; i++) {
    serial_params[i] = pseudonoise (&L_PN_seed, 1);
  }

  return;
}

void GsmEfrWrapper::Decode(const short * efr_dec, int num_frames, char * pcm_out) {
  Word16 *synth;              /* Synthesis                  */
  Word16 parm[PRM_SIZE + 1];  /* Synthesis parameters       */
  Word16 serial[SERIAL_SIZE + 2]; /* Serial stream              */
  Word16 Az_dec[AZ_SIZE];     /* Decoded Az for post-filter */
  /* in 4 subframes, length= 44 */
  Word16 i, frame, temp;
  // FILE *f_syn, *f_serial;

  Word16 TAF, SID_flag;

  Word16 reset_flag;
  static Word16 reset_flag_old = 1;

  synth = synth_buf + M;
  reset_dec (); /* Bring the decoder and receive DTX to the initial state */
  /*-----------------------------------------------------------------*
   *            Loop for each "L_FRAME" speech data                  *
   *-----------------------------------------------------------------*/
  frame = 0;
  // while (fread (serial, sizeof (Word16), 247, f_serial) == 247) {
  for (int iframe = 0; iframe < num_frames_; iframe++) {
    std::memcpy(serial, efr_dec + iframe * 247, sizeof(Word16) * 247);
    SID_flag = serial[245];         /* Receive SID flag */
    TAF = serial[246];              /* Receive TAF flag */
    Bits2prm_12k2 (serial, parm);   /* serial to parameters   */
    if (parm[0] == 0) {
      /* BFI == 0, perform DHF check */
      if (reset_flag_old == 1) {
        /* Check for second and further successive DHF (to first subfr.) */
        reset_flag = decoder_homing_frame_test (&parm[1], TO_FIRST_SUBFRAME);
      } else {
        reset_flag = 0;
      }
    } else {
      /* BFI==1, bypass DHF check (frameis taken as not being a DHF) */
      reset_flag = 0;
    }

    if ((reset_flag != 0) && (reset_flag_old != 0)) {
      /* Force the output to be the encoder homing frame pattern */
      for (i = 0; i < L_FRAME; i++) {
        synth[i] = EHF_MASK;
      }
    } else {
      Decoder_12k2 (parm, synth, Az_dec, TAF, SID_flag);/* Synthesis */
      Post_Filter (synth, Az_dec);                      /* Post-filter */
      for (i = 0; i < L_FRAME; i++) {
        /* Upscale the 15 bit linear PCM to 16 bits, sthen truncate to 13 bits */
        temp = efr_shl (synth[i], 1);
        synth[i] = temp & 0xfff8;       logic16 (); move16 ();
      }
    }                       /* else */

    // fwrite (synth, sizeof (Word16), L_FRAME, f_syn);
    std::memcpy(pcm_out + iframe * L_FRAME * sizeof(Word16), synth, sizeof(Word16) * L_FRAME);

    /* BFI == 0, perform check for first DHF (whole frame) */
    if ((parm[0] == 0) && (reset_flag_old == 0)) {
      reset_flag = decoder_homing_frame_test (&parm[1], WHOLE_FRAME);
    }

    if (reset_flag != 0) {
      /* Bring the decoder and receive DTX to the home state */
      reset_dec ();
    }
    reset_flag_old = reset_flag;
  }                           /* while */

}

//convert the encoded format of EFR to the decoder format
void GsmEfrWrapper::EncodeToDecode(const short * efr_enc, short * efr_dec) {
#define SPEECH      1
#define CNIFIRSTSID 2
#define CNICONT     3
#define VALIDSID    11
#define GOODSPEECH  33

  static Word16 decoding_mode = {SPEECH};
  static Word16 TAF_count = {1};
  Word16 serial_in_para[246], i, frame_type;
  Word16 serial_out_para[247];

  for (int iframe = 0; iframe < num_frames_; iframe++) {
    std::memcpy(serial_in_para, efr_enc + iframe * 246, sizeof(Word16) * 246);
    // if (encoder_interface (infile, serial_in_para) != 0) {
    //   return ;
    // }

    /* Copy input parameters to output parameters */
    /* ------------------------------------------ */
    for (i = 0; i < 244; i++) {
      serial_out_para[i + 1] = serial_in_para[i];
    }

    /* Set channel status (BFI) flag to zero */
    /* --------------------------------------*/
    serial_out_para[0] = 0;     /* BFI flag */

    /* Evaluate SID flag                                  */
    /* Function sid_frame_detection() is defined in dtx.c */
    /* -------------------------------------------------- */
    serial_out_para[245] = sid_frame_detection (&serial_out_para[1]);

    /* Evaluate TAF flag */
    /* ----------------- */
    if (TAF_count == 0) {
      serial_out_para[246] = 1;
    } else {
      serial_out_para[246] = 0;
    }

    TAF_count = (TAF_count + 1) % 24;

    /* Frame classification:                                                */
    /* Since the transmission is error free, the received frames are either */
    /* valid speech or valid SID frames                                     */
    /* -------------------------------------------------------------------- */
    if (serial_out_para[245] == 2) {
      frame_type = VALIDSID;
    } else if (serial_out_para[245] == 0) {
      frame_type = GOODSPEECH;
    } else {
      fprintf (stderr, "Error in SID detection\n");
      return ;
    }

    /* Update of decoder state */
    /* ----------------------- */
    if (decoding_mode == SPEECH) { /* State of previous frame */
      if (frame_type == VALIDSID) {
        decoding_mode = CNIFIRSTSID;
      } else if (frame_type == GOODSPEECH) {
        decoding_mode = SPEECH;
      }
    } else { /* comfort noise insertion mode */
      if (frame_type == VALIDSID) {
        decoding_mode = CNICONT;
      } else if (frame_type == GOODSPEECH) {
        decoding_mode = SPEECH;
      }
    }

    /* Replace parameters by random data if in CNICONT-mode and TAF=0 */
    /* -------------------------------------------------------------- */
    if ((decoding_mode == CNICONT) && (serial_out_para[246] == 0)) {
      random_parameters (&serial_out_para[1]);

      /* Set flags such that an "unusable frame" is produced */
      serial_out_para[0] = 1;       /* BFI flag */
      serial_out_para[245] = 0;     /* SID flag */
    }

    std::memcpy(efr_dec + iframe * 247, serial_out_para, sizeof(Word16) * 247);
    // if (decoder_interface (serial_out_para, outfile) != 0) {
    //   fprintf (stderr, "Error writing File\n");
    //   return 0;
    // }
  }
}

void GsmEfrWrapper::Simulate(const char * pcm_in, int in_samples, char * pcm_out) {
  num_frames_ = in_samples / samples_per_frame_;
  short *efr_enc = new short [num_frames_ * 246];
  short *efr_dec = new short[num_frames_ * 247];

  Encode(pcm_in, in_samples, efr_enc);
  EncodeToDecode(efr_enc, efr_dec);
  Decode(efr_dec, num_frames_, pcm_out);
  if (efr_enc != nullptr) {delete []efr_enc; efr_enc = nullptr;}
  if (efr_dec != nullptr) {delete []efr_dec; efr_dec = nullptr;}
};