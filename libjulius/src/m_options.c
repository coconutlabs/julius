/**
 * @file   m_options.c
 * 
 * <JA>
 * @brief  ���ץ�������
 *
 * �����ˤ���ؿ��ϡ�jconf�ե����뤪��ӥ��ޥ�ɥ饤�󤫤�Υ��ץ��������
 * ����ɤ߹��ߡ��ͤ��Ǽ����. 
 * </JA>
 * 
 * <EN>
 * @brief  Option parsing.
 *
 * These functions read option strings from jconf file or command line
 * and set values to the configuration structure.
 * </EN>
 * 
 * @author Akinobu Lee
 * @date   Thu May 12 18:52:07 2005
 *
 * $Revision: 1.11 $
 * 
 */
/*
 * Copyright (c) 1991-2007 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2007 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <julius/julius.h>

/** 
 * <JA>
 * @brief  ���Хѥ���ե�ѥ����Ѵ�����. 
 * 
 * �ե�����Υѥ�̾�����Хѥ��Ǥ���С������ȥǥ��쥯�ȥ��Ĥ���
 * �ե�ѥ����Ѵ������֤�. ���Хѥ��Ǥ���С����Τޤ��֤�. 
 * 
 * @param filename [in] �ե�����Υѥ�̾
 * @param dirname [in] �����ȥǥ��쥯�ȥ�Υѥ�̾
 * 
 * @return ���Хѥ�̾�����ä��������˳���դ���줿�Хåե�
 * </JA>
 * <EN>
 * @brief  Change relative path to full path.
 *
 * If the file path is given as relative, prepend the dirname to it.
 * If the file path is full, just copy it to new buffer and return.
 * 
 * @param filename [in] file path name
 * @param dirname [in] full path of current directory
 * 
 * @return newly malloced buffer holding the full path name.
 * </EN>
 */
char *
filepath(char *filename, char *dirname)
{
  char *p;
  if (dirname != NULL && filename[0] != '/'
#if defined(_WIN32)
      && filename[0] != '\\' && !(strlen(filename) >= 3 && filename[1] == ':')
#endif
      ) {
    p = (char *)mymalloc(strlen(filename) + strlen(dirname) + 1);
    strcpy(p, dirname);
    strcat(p, filename);
  } else {
    p = strcpy((char *)mymalloc(strlen(filename)+1), filename);
  }
  return p;
}

/** 
 * <EN>
 * Returns next argument string.
 * </EN>
 * <JA>
 * ���ΰ�����ʸ������֤�. 
 * </JA>
 * 
 * @param cur [i/o] pointer to current point of the argment array
 * @param argc [in] total number of argments
 * @param argv [in] argment array
 * 
 * @return pointer to the next argument, or NULL if no more argument vailable.
 * 
 */
static char *
next_arg(int *cur, int argc, char *argv[])
{
  (*cur)++;
  if (*cur >= argc) {
    jlog("ERROR: m_options: option requires argument -- %s\n", argv[*cur-1]);
    return NULL;
  }
  return(argv[*cur]);
}

/** 
 * <JA>
 * �����ΰ������� NULL ������. 
 * @param p [i/o] �����ΰ����Ƭ��ؤ��ݥ����ѿ��ؤΥݥ���
 * @note @a p �� NULL �ξ��ϲ��ⵯ����ʤ���
 * </JA>
 * <EN>
 * Free memory and fill it with NULL.
 * @param p [i/o] pointer to pointer that holds allocated address
 * @note Nothing will happen if @a p equals to NULL.
 * </EN>
 */
#define FREE_MEMORY(p) \
  {if (p) {free(p); p = NULL;}}

/**
 * <JA>
 * ���ץ�������.
 *
 * @param argc [in] @a argv �˴ޤޤ������ο�
 * @param argv [in] �����͡�ʸ����ˤ�����
 * @param cwd [in] �����ȥǥ��쥯�ȥ�
 * @param jconf [out] �ͤ��Ǽ����jconf��¤��
 * 
 * </JA>
 * <EN>
 * Option parsing.
 * 
 * @param argc [in] number of elements in @a argv
 * @param argv [in] array of argument strings
 * @param cwd [in] current directory
 * @param jconf [out] jconf structure to store data
 * 
 * </EN>
 * @return TRUE on success, or FALSE on error.
 *
 * @callgraph
 * @callergraph
 */
boolean
opt_parse(int argc, char *argv[], char *cwd, Jconf *jconf)
{
  char *tmparg;
  int i;
  boolean unknown_opt;
  JCONF_AM *amconf, *atmp;
  JCONF_LM *lmconf, *ltmp;
  JCONF_SEARCH *sconf, *stmp;
  char sname[JCONF_MODULENAME_MAXLEN];
#define GET_TMPARG  if ((tmparg = next_arg(&i, argc, argv)) == NULL) return FALSE

  for (i=1;i<argc;i++) {
    unknown_opt = FALSE;
    if (strmatch(argv[i],"-C")) { /* include jconf file  */
      GET_TMPARG;
      tmparg = filepath(tmparg, cwd);
      if (config_file_parse(tmparg, jconf) == FALSE) {
	return FALSE;
      }
      free(tmparg);
      continue;
    } else if (strmatch(argv[i],"-AM") || strmatch(argv[i], "[AM]")) {
      GET_TMPARG;
      if (tmparg[0] == '-') {
	jlog("ERROR: m_options: -AM needs an argument as module name\n");
	return FALSE;
      }
      if (tmparg[0] >= '0' && tmparg[0] <= '9') {
	jlog("ERROR: m_options: AM name \"%s\" not acceptable: first character should not be a digit\n", tmparg);
	return FALSE;
      }
      /* if not first time, create new module instance and switch to it */
      /* and switch current to this */
      amconf = j_jconf_am_new();
      if (j_jconf_am_regist(jconf, amconf, tmparg) == FALSE) {
	jlog("ERROR: failed to add new amconf as \"%s\"\n", tmparg);
	jlog("ERROR: m_options: failed to create amconf\n");
	j_jconf_am_free(amconf);
	return FALSE;
      }
      jconf->amnow = amconf;
      continue;
    } else if (strmatch(argv[i],"-AM_GMM") || strmatch(argv[i], "[AM_GMM]")) {
      /* switch current to GMM */
      if (jconf->gmm == NULL) {
	/* if new, allocate jconf for GMM */
	jconf->gmm = j_jconf_am_new();
      }
      jconf->amnow = jconf->gmm;
      continue;
    } else if (strmatch(argv[i],"-LM") || strmatch(argv[i], "[LM]")) {
      GET_TMPARG;
      if (tmparg[0] == '-') {
	jlog("ERROR: m_options: -LM needs an argument as module name\n");
	return FALSE;
      }
      if (tmparg[0] >= '0' && tmparg[0] <= '9') {
	jlog("ERROR: m_options: LM name \"%s\" not acceptable: first character should not be a digit\n", tmparg);
	return FALSE;
      }
      /* create new module instance and switch to it */
      /* and switch current to this */
      lmconf = j_jconf_lm_new();
      if (j_jconf_lm_regist(jconf, lmconf, tmparg) == FALSE) {
	jlog("ERROR: failed to add new lmconf as \"%s\"\n", tmparg);
	jlog("ERROR: m_options: failed to create lmconf\n");
	j_jconf_lm_free(lmconf);
	return FALSE;
      }
      jconf->lmnow = lmconf;
      continue;
    } else if (strmatch(argv[i],"-SR") || strmatch(argv[i], "[SR]")) {
      GET_TMPARG;
      if (tmparg[0] == '-') {
	jlog("ERROR: m_options: -SR needs three arguments: module name, AM name and LM name\n");
	return FALSE;
      }
      if (tmparg[0] >= '0' && tmparg[0] <= '9') {
	jlog("ERROR: m_options: SR name \"%s\" not acceptable: first character should not be a digit\n", tmparg);
	return FALSE;
      }
      /* store name temporarly */
      strncpy(sname, tmparg, JCONF_MODULENAME_MAXLEN);
      /* get link to jconf_am and jconf_lm */
      GET_TMPARG;
      if (tmparg[0] == '-') {
	jlog("ERROR: m_options: -SR needs three arguments: module name, AM name and LM name\n");
	return FALSE;
      }
      if (tmparg[0] >= '0' && tmparg[0] <= '9') { /* arg is number */
	if ((amconf = j_get_amconf_by_id(jconf, atoi(tmparg))) == NULL) return FALSE;
      } else {			/* name string */
	if ((amconf = j_get_amconf_by_name(jconf, tmparg)) == NULL) return FALSE;
      }
      GET_TMPARG;
      if (tmparg[0] == '-') {
	jlog("ERROR: m_options: -SR needs three arguments: module name, AM name and LM name\n");
	return FALSE;
      }
      if (tmparg[0] >= '0' && tmparg[0] <= '9') { /* arg is number */
	if ((lmconf = j_get_lmconf_by_id(jconf, atoi(tmparg))) == NULL) return FALSE;
      } else {			/* name string */
	if ((lmconf = j_get_lmconf_by_name(jconf, tmparg)) == NULL) return FALSE;
      }

      /* if not first time, create new module instance and switch to it */
      sconf = j_jconf_search_new();
      sconf->amconf = amconf;
      sconf->lmconf = lmconf;
      if (j_jconf_search_regist(jconf, sconf, sname) == FALSE) {
	jlog("ERROR: failed to add new amconf as \"%s\"\n", sname);
	jlog("ERROR: m_options: failed to create search conf\n");
	j_jconf_search_free(sconf);
	return FALSE;
      }
      jconf->searchnow = sconf;
      continue;
    } else if (strmatch(argv[i],"-input")) { /* speech input */
      GET_TMPARG;
      if (strmatch(tmparg,"file")) {
	jconf->input.speech_input = SP_RAWFILE;
	jconf->decodeopt.realtime_flag = FALSE;
      } else if (strmatch(tmparg,"rawfile")) {
	jconf->input.speech_input = SP_RAWFILE;
	jconf->decodeopt.realtime_flag = FALSE;
      } else if (strmatch(tmparg,"htkparam")) {
	jconf->input.speech_input = SP_MFCFILE;
	jconf->decodeopt.realtime_flag = FALSE;
      } else if (strmatch(tmparg,"mfcfile")) {
	jconf->input.speech_input = SP_MFCFILE;
	jconf->decodeopt.realtime_flag = FALSE;
      } else if (strmatch(tmparg,"stdin")) {
	jconf->input.speech_input = SP_STDIN;
	jconf->decodeopt.realtime_flag = FALSE;
      } else if (strmatch(tmparg,"adinnet")) {
	jconf->input.speech_input = SP_ADINNET;
	jconf->decodeopt.realtime_flag = TRUE;
#ifdef USE_NETAUDIO
      } else if (strmatch(tmparg,"netaudio")) {
	jconf->input.speech_input = SP_NETAUDIO;
	jconf->decodeopt.realtime_flag = TRUE;
#endif
#ifdef USE_MIC
      } else if (strmatch(tmparg,"mic")) {
	jconf->input.speech_input = SP_MIC;
	jconf->input.device = SP_INPUT_DEFAULT;
	jconf->decodeopt.realtime_flag = TRUE;
      } else if (strmatch(tmparg,"alsa")) {
#ifdef HAS_ALSA
	jconf->input.speech_input = SP_MIC;
	jconf->input.device = SP_INPUT_ALSA;
	jconf->decodeopt.realtime_flag = TRUE;
#else
	jlog("ERROR: m_options: \"-input alsa\": ALSA support is not built-in\n");
	return FALSE;
#endif
      } else if (strmatch(tmparg,"oss")) {
#ifdef HAS_OSS
	jconf->input.speech_input = SP_MIC;
	jconf->input.device = SP_INPUT_OSS;
	jconf->decodeopt.realtime_flag = TRUE;
#else
	jlog("ERROR: m_options: \"-input oss\": OSS support is not built-in\n");
	return FALSE;
#endif
      } else if (strmatch(tmparg,"esd")) {
#ifdef HAS_ESD
	jconf->input.speech_input = SP_MIC;
	jconf->input.device = SP_INPUT_ESD;
	jconf->decodeopt.realtime_flag = TRUE;
#else
	jlog("ERROR: m_options: \"-input oss\": OSS support is not built-in\n");
	return FALSE;
#endif
#endif
      } else if (strmatch(tmparg,"file")) { /* for 1.1 compat */
	jconf->input.speech_input = SP_RAWFILE;
	jconf->decodeopt.realtime_flag = FALSE;
      } else if (strmatch(tmparg,"mfc")) { /* for 1.1 compat */
	jconf->input.speech_input = SP_MFCFILE;
	jconf->decodeopt.realtime_flag = FALSE;
      } else {
	jlog("ERROR: m_options: unknown speech input source \"%s\"\n", tmparg);
	return FALSE;
      }
      continue;
    } else if (strmatch(argv[i],"-filelist")) {	/* input file list */
      FREE_MEMORY(jconf->input.inputlist_filename);
      GET_TMPARG;
      //jconf->input.inputlist_filename = strcpy((char*)mymalloc(strlen(tmparg)+1),tmparg);
      jconf->input.inputlist_filename = filepath(tmparg, cwd);
      continue;
    } else if (strmatch(argv[i],"-rejectshort")) { /* short input rejection */
      GET_TMPARG;
      jconf->reject.rejectshortlen = atoi(tmparg);
      continue;
#ifdef POWER_REJECT
    } else if (strmatch(argv[i],"-powerthres")) { /* short input rejection */
      GET_TMPARG;
      jconf->reject.powerthres = atoi(tmparg);
      continue;
#endif
    } else if (strmatch(argv[i],"-force_realtime")) { /* force realtime */
      GET_TMPARG;
      if (strmatch(tmparg, "on")) {
	jconf->decodeopt.forced_realtime = TRUE;
      } else if (strmatch(tmparg, "off")) {
	jconf->decodeopt.forced_realtime = FALSE;
      } else {
	jlog("ERROR: m_options: \"-force_realtime\" should be either \"on\" or \"off\"\n");
	return FALSE;
      }
      jconf->decodeopt.force_realtime_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-realtime")) {	/* equal to "-force_realtime on" */
      jconf->decodeopt.forced_realtime = TRUE;
      jconf->decodeopt.force_realtime_flag = TRUE;
      continue;
    } else if (strmatch(argv[i], "-norealtime")) { /* equal to "-force_realtime off" */
      jconf->decodeopt.forced_realtime = FALSE;
      jconf->decodeopt.force_realtime_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-forcedict")) { /* skip dict error */
      jconf->lmnow->forcedict_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-check")) { /* interactive model check mode */
      GET_TMPARG;
      if (strmatch(tmparg, "wchmm")) {
	jconf->searchnow->sw.wchmm_check_flag = TRUE;
      } else if (strmatch(tmparg, "trellis")) {
	jconf->searchnow->sw.trellis_check_flag = TRUE;
      } else if (strmatch(tmparg, "triphone")) {
	jconf->searchnow->sw.triphone_check_flag = TRUE;
      } else {
	jlog("ERROR: m_options: invalid argument for \"-check\": %s\n", tmparg);
	return FALSE;
      }
      continue;
    } else if (strmatch(argv[i],"-notypecheck")) { /* don't check param type */
      jconf->input.paramtype_check_flag = FALSE;
      continue;
    } else if (strmatch(argv[i],"-nlimit")) { /* limit N token in a node */
#ifdef WPAIR_KEEP_NLIMIT
      GET_TMPARG;
      jconf->searchnow->pass1.wpair_keep_nlimit = atoi(tmparg);
#else
      jlog("WARNING: m_options: WPAIR_KEEP_NLIMIT disabled, \"-nlimit\" ignored\n");
#endif
      continue;
    } else if (strmatch(argv[i],"-lookuprange")) { /* trellis neighbor range */
      GET_TMPARG;
      jconf->searchnow->pass2.lookup_range = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-graphout")) { /* enable graph output */
      jconf->searchnow->graph.enabled = TRUE;
      jconf->searchnow->graph.lattice = TRUE;
      jconf->searchnow->graph.confnet = FALSE;
      continue;
    } else if (strmatch(argv[i],"-lattice")) { /* enable graph output */
      jconf->searchnow->graph.enabled = TRUE;
      jconf->searchnow->graph.lattice = TRUE;
      continue;
    } else if (strmatch(argv[i],"-nolattice")) { /* disable graph output */
      jconf->searchnow->graph.enabled = FALSE;
      jconf->searchnow->graph.lattice = FALSE;
      continue;
    } else if (strmatch(argv[i],"-confnet")) { /* enable confusion network */
      jconf->searchnow->graph.enabled = TRUE;
      jconf->searchnow->graph.confnet = TRUE;
      continue;
    } else if (strmatch(argv[i],"-noconfnet")) { /* disable graph output */
      jconf->searchnow->graph.enabled = FALSE;
      jconf->searchnow->graph.confnet = FALSE;
      continue;
    } else if (strmatch(argv[i],"-graphrange")) { /* neighbor merge range frame */
      GET_TMPARG;
      jconf->searchnow->graph.graph_merge_neighbor_range = atoi(tmparg);
      continue;
#ifdef GRAPHOUT_DEPTHCUT
    } else if (strmatch(argv[i],"-graphcut")) { /* cut graph word by depth */
      GET_TMPARG;
      jconf->searchnow->graph.graphout_cut_depth = atoi(tmparg);
      continue;
#endif
#ifdef GRAPHOUT_LIMIT_BOUNDARY_LOOP
    } else if (strmatch(argv[i],"-graphboundloop")) { /* neighbor merge range frame */
      GET_TMPARG;
      jconf->searchnow->graph.graphout_limit_boundary_loop_num = atoi(tmparg);
      continue;
#endif
#ifdef GRAPHOUT_SEARCH_DELAY_TERMINATION
    } else if (strmatch(argv[i],"-graphsearchdelay")) { /* not do graph search termination before the 1st sentence is found */
      jconf->searchnow->graph.graphout_search_delay = TRUE;
      continue;
    } else if (strmatch(argv[i],"-nographsearchdelay")) { /* not do graph search termination before the 1st sentence is found */
      jconf->searchnow->graph.graphout_search_delay = FALSE;
      continue;
#endif
    } else if (strmatch(argv[i],"-looktrellis")) { /* activate loopuprange */
      jconf->searchnow->pass2.looktrellis_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-multigramout")) { /* enable per-grammar decoding on 2nd pass */
      jconf->searchnow->output.multigramout_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-nomultigramout")) { /* disable per-grammar decoding on 2nd pass */
      jconf->searchnow->output.multigramout_flag = FALSE;
      continue;
    } else if (strmatch(argv[i],"-oldtree")) { /* use old tree function */
      jconf->searchnow->pass1.old_tree_function_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-sb")) { /* score envelope width in 2nd pass */
#ifdef SCAN_BEAM
      GET_TMPARG;
      jconf->searchnow->pass2.scan_beam_thres = atof(tmparg);
#else
      jlog("WARNING: m_options: SCAN_BEAM disabled, \"-sb\" ignored\n");
#endif
      continue;
    } else if (strmatch(argv[i],"-discount")) {	/* (bogus) */
      jlog("WARNING: m_options: option \"-discount\" is now bogus, ignored\n");
      continue;
    } else if (strmatch(argv[i],"-cutsilence")) { /* force (long) silence detection on */
      jconf->detect.silence_cut = 1;
      continue;
    } else if (strmatch(argv[i],"-nocutsilence")) { /* force (long) silence detection off */
      jconf->detect.silence_cut = 0;
      continue;
    } else if (strmatch(argv[i],"-pausesegment")) { /* force (long) silence detection on (for backward compatibility) */
      jconf->detect.silence_cut = 1;
      continue;
    } else if (strmatch(argv[i],"-nopausesegment")) { /* force (long) silence detection off (for backward comatibility) */
      jconf->detect.silence_cut = 0;
      continue;
    } else if (strmatch(argv[i],"-lv")) { /* silence detection threshold level */
      GET_TMPARG;
      jconf->detect.level_thres = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-zc")) { /* silence detection zero cross num */
      GET_TMPARG;
      jconf->detect.zero_cross_num = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-headmargin")) { /* head silence length */
      GET_TMPARG;
      jconf->detect.head_margin_msec = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-tailmargin")) { /* tail silence length */
      GET_TMPARG;
      jconf->detect.tail_margin_msec = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-hipass")||strmatch(argv[i],"-hifreq")) { /* frequency of upper band limit */
      GET_TMPARG;
      jconf->amnow->analysis.para.hipass = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-lopass")||strmatch(argv[i],"-lofreq")) { /* frequency of lower band limit */
      GET_TMPARG;
      jconf->amnow->analysis.para.lopass = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-smpPeriod")) { /* sample period (ns) */
      GET_TMPARG;
      jconf->amnow->analysis.para.smp_period = atoi(tmparg);
      jconf->amnow->analysis.para.smp_freq = period2freq(jconf->amnow->analysis.para.smp_period);
      continue;
    } else if (strmatch(argv[i],"-smpFreq")) { /* sample frequency (Hz) */
      GET_TMPARG;
      jconf->amnow->analysis.para.smp_freq = atoi(tmparg);
      jconf->amnow->analysis.para.smp_period = freq2period(jconf->amnow->analysis.para.smp_freq);
      continue;
    } else if (strmatch(argv[i],"-fsize")) { /* Window size */
      GET_TMPARG;
      jconf->amnow->analysis.para.framesize = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-fshift")) { /* Frame shiht */
      GET_TMPARG;
      jconf->amnow->analysis.para.frameshift = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-preemph")) {
      GET_TMPARG;
      jconf->amnow->analysis.para.preEmph = atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-fbank")) {
      GET_TMPARG;
      jconf->amnow->analysis.para.fbank_num = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-ceplif")) {
      GET_TMPARG;
      jconf->amnow->analysis.para.lifter = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-rawe")) {
      jconf->amnow->analysis.para.raw_e = TRUE;
      continue;
    } else if (strmatch(argv[i],"-norawe")) {
      jconf->amnow->analysis.para.raw_e = FALSE;
      continue;
    } else if (strmatch(argv[i],"-enormal")) {
      jconf->amnow->analysis.para.enormal = TRUE;
      continue;
    } else if (strmatch(argv[i],"-noenormal")) {
      jconf->amnow->analysis.para.enormal = FALSE;
      continue;
    } else if (strmatch(argv[i],"-escale")) {
      GET_TMPARG;
      jconf->amnow->analysis.para.escale = atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-silfloor")) {
      GET_TMPARG;
      jconf->amnow->analysis.para.silFloor = atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-delwin")) { /* Delta window length */
      GET_TMPARG;
      jconf->amnow->analysis.para.delWin = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-accwin")) { /* Acceleration window length */
      GET_TMPARG;
      jconf->amnow->analysis.para.accWin = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-ssalpha")) { /* alpha coef. for SS */
      GET_TMPARG;
      jconf->amnow->frontend.ss_alpha = atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-ssfloor")) { /* spectral floor for SS */
      GET_TMPARG;
      jconf->amnow->frontend.ss_floor = atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-cvn")) {
      jconf->amnow->analysis.para.cvn = 1;
      continue;
    } else if (strmatch(argv[i],"-nocvn")) {
      jconf->amnow->analysis.para.cvn = 0;
      continue;
    } else if (strmatch(argv[i],"-vtln")) { /* VTLN */
      GET_TMPARG;
      jconf->amnow->analysis.para.vtln_alpha = (float)atof(tmparg);
      GET_TMPARG;
      jconf->amnow->analysis.para.vtln_lower = (float)atof(tmparg);
      GET_TMPARG;
      jconf->amnow->analysis.para.vtln_upper = (float)atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-novtln")) { /* disable VTLN */
      jconf->amnow->analysis.para.vtln_alpha = 1.0;
      continue;
    } else if (strmatch(argv[i],"-48")) { /* use 48kHz input and down to 16kHz */
      jconf->input.use_ds48to16 = TRUE;
      continue;
    } else if (strmatch(argv[i],"-version") || strmatch(argv[i], "--version") || strmatch(argv[i], "-setting") || strmatch(argv[i], "--setting")) { /* print version and exit */
      j_put_header(stderr);
      j_put_compile_defs(stderr);
      fprintf(stderr, "\n");
      j_put_library_defs(stderr);
      return FALSE;
    } else if (strmatch(argv[i],"-quiet")) { /* minimum output */
      debug2_flag = verbose_flag = FALSE;
      continue;
    } else if (strmatch(argv[i],"-debug")) { /* debug mode: output huge log */
      debug2_flag = verbose_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-callbackdebug")) { /* output callback debug message */
      callback_debug_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-progout")) { /* enable progressive output */
      jconf->searchnow->output.progout_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-proginterval")) { /* interval for -progout */
      GET_TMPARG;
      jconf->searchnow->output.progout_interval = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-demo")) { /* quiet + progout */
      debug2_flag = verbose_flag = FALSE;
      jconf->searchnow->output.progout_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-walign")) { /* do forced alignment by word */
      jconf->searchnow->annotate.align_result_word_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-palign")) { /* do forced alignment by phoneme */
      jconf->searchnow->annotate.align_result_phoneme_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-salign")) { /* do forced alignment by state */
      jconf->searchnow->annotate.align_result_state_flag = TRUE;
      continue;
    } else if (strmatch(argv[i],"-output")) { /* output up to N candidate */
      GET_TMPARG;
      jconf->searchnow->output.output_hypo_maxnum = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-1pass")) { /* do only 1st pass */
      jconf->searchnow->compute_only_1pass = TRUE;
      continue;
    } else if (strmatch(argv[i],"-hlist")) { /* HMM list file */
      FREE_MEMORY(jconf->amnow->mapfilename);
      GET_TMPARG;
      jconf->amnow->mapfilename = filepath(tmparg, cwd);
      continue;
    } else if (strmatch(argv[i],"-nlr")) { /* word LR n-gram (ARPA) */
      FREE_MEMORY(jconf->lmnow->ngram_filename_lr_arpa);
      GET_TMPARG;
      jconf->lmnow->ngram_filename_lr_arpa = filepath(tmparg, cwd);
      FREE_MEMORY(jconf->lmnow->ngram_filename);
      continue;
    } else if (strmatch(argv[i],"-nrl")) { /* word RL n-gram (ARPA) */
      FREE_MEMORY(jconf->lmnow->ngram_filename_rl_arpa);
      GET_TMPARG;
      jconf->lmnow->ngram_filename_rl_arpa = filepath(tmparg, cwd);
      FREE_MEMORY(jconf->lmnow->ngram_filename);
      continue;
    } else if (strmatch(argv[i],"-lmp")) { /* LM weight and penalty (pass1) */
      GET_TMPARG;
      jconf->searchnow->lmp.lm_weight = (LOGPROB)atof(tmparg);
      GET_TMPARG;
      jconf->searchnow->lmp.lm_penalty = (LOGPROB)atof(tmparg);
      jconf->searchnow->lmp.lmp_specified = TRUE;
      continue;
    } else if (strmatch(argv[i],"-lmp2")) { /* LM weight and penalty (pass2) */
      GET_TMPARG;
      jconf->searchnow->lmp.lm_weight2 = (LOGPROB)atof(tmparg);
      GET_TMPARG;
      jconf->searchnow->lmp.lm_penalty2 = (LOGPROB)atof(tmparg);
      jconf->searchnow->lmp.lmp2_specified = TRUE;
      continue;
    } else if (strmatch(argv[i],"-transp")) { /* penalty for transparent word */
      GET_TMPARG;
      jconf->searchnow->lmp.lm_penalty_trans = (LOGPROB)atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-gram")) { /* comma-separatedlist of grammar prefix */
      GET_TMPARG;
      if (multigram_add_prefix_list(tmparg, cwd, jconf->lmnow, LM_DFA_GRAMMAR) == FALSE) {
	jlog("ERROR: m_options: failed to read some grammars\n");
	return FALSE;
      }
      continue;
    } else if (strmatch(argv[i],"-gramlist")) { /* file of grammar prefix list */
      GET_TMPARG;
      tmparg = filepath(tmparg, cwd);
      if (multigram_add_prefix_filelist(tmparg, jconf->lmnow, LM_DFA_GRAMMAR) == FALSE) {
	jlog("ERROR: m_options: failed to read some grammars\n");
	free(tmparg);
	return FALSE;
      }
      free(tmparg);
      continue;
    } else if (strmatch(argv[i],"-userlm")) {
      /* just set lm flags here */
      if (jconf->lmnow->lmtype != LM_PROB && jconf->lmnow->lmtype != LM_UNDEF) {
	jlog("ERROR: m_options: LM type conflicts: multiple LM specified?\n");
	return FALSE;
      }
      jconf->lmnow->lmtype = LM_PROB;
      if (jconf->lmnow->lmvar != LM_UNDEF && jconf->lmnow->lmvar != LM_NGRAM_USER) {
	jlog("ERROR: m_options: statistical model conflict\n");
	return FALSE;
      }
      jconf->lmnow->lmvar  = LM_NGRAM_USER;
      continue;
    } else if (strmatch(argv[i],"-nogram")) { /* remove grammar list */
      multigram_remove_gramlist(jconf->lmnow);
      FREE_MEMORY(jconf->lmnow->dfa_filename);
      FREE_MEMORY(jconf->lmnow->dictfilename);
      continue;
    } else if (strmatch(argv[i],"-dfa")) { /* DFA filename */
      FREE_MEMORY(jconf->lmnow->dfa_filename);
      GET_TMPARG;
      jconf->lmnow->dfa_filename = filepath(tmparg, cwd);
      continue;
    } else if (strmatch(argv[i],"-penalty1")) {	/* word insertion penalty (pass1) */
      GET_TMPARG;
      jconf->searchnow->lmp.penalty1 = (LOGPROB)atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-penalty2")) {	/* word insertion penalty (pass2) */
      GET_TMPARG;
      jconf->searchnow->lmp.penalty2 = (LOGPROB)atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-spmodel") || strmatch(argv[i], "-sp")) { /* name of short pause word */
      FREE_MEMORY(jconf->amnow->spmodel_name);
      GET_TMPARG;
      jconf->amnow->spmodel_name = strcpy((char*)mymalloc(strlen(tmparg)+1),tmparg);
      continue;
    } else if (strmatch(argv[i],"-multipath")) { /* force multipath mode */
      jconf->amnow->force_multipath = TRUE;
      continue;
    } else if (strmatch(argv[i],"-iwsp")) { /* enable inter-word short pause handing (for multipath) */
      jconf->lmnow->enable_iwsp = TRUE;
      continue;
    } else if (strmatch(argv[i],"-iwsppenalty")) { /* set inter-word short pause transition penalty (for multipath) */
      GET_TMPARG;
      jconf->amnow->iwsp_penalty = atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-silhead")) { /* head silence word name */
      FREE_MEMORY(jconf->lmnow->head_silname);
      GET_TMPARG;
      jconf->lmnow->head_silname = strcpy((char*)mymalloc(strlen(tmparg)+1),tmparg);
      continue;
    } else if (strmatch(argv[i],"-siltail")) { /* tail silence word name */
      FREE_MEMORY(jconf->lmnow->tail_silname);
      GET_TMPARG;
      jconf->lmnow->tail_silname = strcpy((char*)mymalloc(strlen(tmparg)+1),tmparg);
      continue;
    } else if (strmatch(argv[i],"-iwspword")) { /* add short pause word */
      jconf->lmnow->enable_iwspword = TRUE;
      continue;
    } else if (strmatch(argv[i],"-iwspentry")) { /* content of the iwspword */
      FREE_MEMORY(jconf->lmnow->iwspentry);
      GET_TMPARG;
      jconf->lmnow->iwspentry = strcpy((char*)mymalloc(strlen(tmparg)+1),tmparg);
      continue;
    } else if (strmatch(argv[i],"-iwcache")) { /* control cross-word LM cache */
#ifdef HASH_CACHE_IW
      GET_TMPARG;
      jconf->searchnow->pass1.iw_cache_rate = atof(tmparg);
      if (jconf->searchnow->pass1.iw_cache_rate > 100) jconf->searchnow->pass1.iw_cache_rate = 100;
      if (jconf->searchnow->pass1.iw_cache_rate < 1) jconf->searchnow->pass1.iw_cache_rate = 1;
#else
      jlog("WARNING: m_options: HASH_CACHE_IW disabled, \"-iwcache\" ignored\n");
#endif
      continue;
    } else if (strmatch(argv[i],"-sepnum")) { /* N-best frequent word will be separated from tree */
#ifdef SEPARATE_BY_UNIGRAM
      GET_TMPARG;
      jconf->lmnow->separate_wnum = atoi(tmparg);
#else
      jlog("WARNING: m_options: SEPARATE_BY_UNIGRAM disabled, \"-sepnum\" ignored\n");
      i++;
#endif
      continue;
#ifdef USE_NETAUDIO
    } else if (strmatch(argv[i],"-NA")) { /* netautio device name */
      FREE_MEMORY(jconf->input.netaudio_devname);
      GET_TMPARG;
      jconf->input.netaudio_devname = strcpy((char*)mymalloc(strlen(tmparg)+1),tmparg);
      continue;
#endif
    } else if (strmatch(argv[i],"-adport")) { /* adinnet port num */
      GET_TMPARG;
      jconf->input.adinnet_port = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-nostrip")) { /* do not strip zero samples */
      jconf->preprocess.strip_zero_sample = FALSE;
      continue;
    } else if (strmatch(argv[i],"-zmean")) { /* enable DC offset by zero mean */
      jconf->preprocess.use_zmean = TRUE;
      continue;
    } else if (strmatch(argv[i],"-nozmean")) { /* disable DC offset by zero mean */
      jconf->preprocess.use_zmean = FALSE;
      continue;
    } else if (strmatch(argv[i],"-zmeanframe")) { /* enable frame-wise DC offset by zero mean */
      jconf->amnow->analysis.para.zmeanframe = TRUE;
      continue;
    } else if (strmatch(argv[i],"-nozmeanframe")) { /* disable frame-wise DC offset by zero mean */
      jconf->amnow->analysis.para.zmeanframe = FALSE;
      continue;
    } else if (strmatch(argv[i],"-usepower")) { /* use power instead of magnitude in filterbank analysis */
      jconf->amnow->analysis.para.usepower = TRUE;
      continue;
    } else if (strmatch(argv[i],"-nousepower")) { /* use magnitude in fbank analysis (default)  */
      jconf->amnow->analysis.para.usepower = FALSE;
      continue;
    } else if (strmatch(argv[i],"-spsegment")) { /* enable short-pause segmentation */
      jconf->searchnow->successive.enabled = TRUE;
      continue;
    } else if (strmatch(argv[i],"-spdur")) { /* speech down-trigger duration threshold in frame */
      GET_TMPARG;
      jconf->searchnow->successive.sp_frame_duration = atoi(tmparg);
      continue;
#ifdef SPSEGMENT_NAIST
    } else if (strmatch(argv[i],"-spmargin")) { /* speech up-trigger backstep margin in frame */
      GET_TMPARG;
      jconf->searchnow->successive.sp_margin = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-spdelay")) { /* speech up-trigger delay frame */
      GET_TMPARG;
      jconf->searchnow->successive.sp_delay = atoi(tmparg);
      continue;
#endif
    } else if (strmatch(argv[i],"-pausemodels")) { /* short-pause duration threshold */
      FREE_MEMORY(jconf->searchnow->successive.pausemodelname);
      GET_TMPARG;
      jconf->searchnow->successive.pausemodelname = strcpy((char*)mymalloc(strlen(tmparg)+1),tmparg);
      continue;
    } else if (strmatch(argv[i],"-gprune")) { /* select Gaussian pruning method */
      GET_TMPARG;
      if (strmatch(tmparg,"safe")) { /* safest, slowest */
	jconf->amnow->gprune_method = GPRUNE_SEL_SAFE;
      } else if (strmatch(tmparg,"heuristic")) {
	jconf->amnow->gprune_method = GPRUNE_SEL_HEURISTIC;
      } else if (strmatch(tmparg,"beam")) { /* fastest */
	jconf->amnow->gprune_method = GPRUNE_SEL_BEAM;
      } else if (strmatch(tmparg,"none")) { /* no prune: compute all Gaussian */
	jconf->amnow->gprune_method = GPRUNE_SEL_NONE;
      } else if (strmatch(tmparg,"default")) {
	jconf->amnow->gprune_method = GPRUNE_SEL_UNDEF;
      } else {
	jlog("ERROR: m_options: no such pruning method \"%s\"\n", argv[0], tmparg);
	return FALSE;
      }
      continue;
/* 
 *     } else if (strmatch(argv[i],"-reorder")) {
 *	 result_reorder_flag = TRUE;
 *	 continue;
 */
    } else if (strmatch(argv[i],"-no_ccd")) { /* force triphone handling = OFF */
      jconf->searchnow->ccd_handling = FALSE;
      jconf->searchnow->force_ccd_handling = TRUE;
      continue;
    } else if (strmatch(argv[i],"-force_ccd")) { /* force triphone handling = ON */
      jconf->searchnow->ccd_handling = TRUE;
      jconf->searchnow->force_ccd_handling = TRUE;
      continue;
    } else if (strmatch(argv[i],"-iwcd1")) { /* select cross-word triphone computation method */
      GET_TMPARG;
      if (strmatch(tmparg, "max")) { /* use maximum score in triphone variants */
	jconf->amnow->iwcdmethod = IWCD_MAX;
      } else if (strmatch(tmparg, "avg")) { /* use average in variants */
	jconf->amnow->iwcdmethod = IWCD_AVG;
      } else if (strmatch(tmparg, "best")) { /* use average in variants */
	jconf->amnow->iwcdmethod = IWCD_NBEST;
	GET_TMPARG;
	jconf->amnow->iwcdmaxn = atoi(tmparg);
      } else {
	jlog("ERROR: m_options: -iwcd1: wrong argument (max|avg|best N): %s\n", argv[0], tmparg);
	return FALSE;
      }
      continue;
    } else if (strmatch(argv[i],"-tmix")) { /* num of mixture to select */
      if (i + 1 < argc && isdigit(argv[i+1][0])) {
	jconf->amnow->mixnum_thres = atoi(argv[++i]);
      }
      continue;
    } else if (strmatch(argv[i],"-b2") || strmatch(argv[i],"-bw") || strmatch(argv[i],"-wb")) {	/* word beam width in 2nd pass */
      GET_TMPARG;
      jconf->searchnow->pass2.enveloped_bestfirst_width = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-hgs")) { /* Gaussian selection model file */
      FREE_MEMORY(jconf->amnow->hmm_gs_filename);
      GET_TMPARG;
      jconf->amnow->hmm_gs_filename = filepath(tmparg, cwd);
      continue;
    } else if (strmatch(argv[i],"-booknum")) { /* num of state to select in GS */
      GET_TMPARG;
      jconf->amnow->gs_statenum = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-gshmm")) { /* same as "-hgs" */
      FREE_MEMORY(jconf->amnow->hmm_gs_filename);
      GET_TMPARG;
      jconf->amnow->hmm_gs_filename = filepath(tmparg, cwd);
      continue;
    } else if (strmatch(argv[i],"-gsnum")) { /* same as "-booknum" */
      GET_TMPARG;
      jconf->amnow->gs_statenum = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-cmnload")) { /* load CMN parameter from file */
      FREE_MEMORY(jconf->amnow->analysis.cmnload_filename);
      GET_TMPARG;
      jconf->amnow->analysis.cmnload_filename = filepath(tmparg, cwd);
      continue;
    } else if (strmatch(argv[i],"-cmnsave")) { /* save CMN parameter to file */
      FREE_MEMORY(jconf->amnow->analysis.cmnsave_filename);
      GET_TMPARG;
      jconf->amnow->analysis.cmnsave_filename = filepath(tmparg, cwd);
      continue;
    } else if (strmatch(argv[i],"-cmnupdate")) { /* update CMN parameter */
      jconf->amnow->analysis.cmn_update = TRUE;
      continue;
    } else if (strmatch(argv[i],"-cmnnoupdate")) { /* not update CMN parameter */
      jconf->amnow->analysis.cmn_update = FALSE;
      continue;
    } else if (strmatch(argv[i],"-cmnmapweight")) { /* CMN weight for MAP */
      GET_TMPARG;
      jconf->amnow->analysis.cmn_map_weight = (float)atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-sscalc")) { /* do spectral subtraction (SS) for raw file input */
      jconf->amnow->frontend.sscalc = TRUE;
      FREE_MEMORY(jconf->amnow->frontend.ssload_filename);
      continue;
    } else if (strmatch(argv[i],"-sscalclen")) { /* head silence length used to compute SS (in msec) */
      GET_TMPARG;
      jconf->amnow->frontend.sscalc_len = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-ssload")) { /* load SS parameter from file */
      FREE_MEMORY(jconf->amnow->frontend.ssload_filename);
      GET_TMPARG;
      jconf->amnow->frontend.ssload_filename = filepath(tmparg, cwd);
      jconf->amnow->frontend.sscalc = FALSE;
      continue;
#ifdef CONFIDENCE_MEASURE
    } else if (strmatch(argv[i],"-cmalpha")) { /* CM log score scaling factor */
#ifdef CM_MULTIPLE_ALPHA
      GET_TMPARG;
      jconf->searchnow->annotate.cm_alpha_bgn = (LOGPROB)atof(tmparg);
      GET_TMPARG;
      jconf->searchnow->annotate.cm_alpha_end = (LOGPROB)atof(tmparg);
      GET_TMPARG;
      jconf->searchnow->annotate.cm_alpha_step = (LOGPROB)atof(tmparg);
      jconf->searchnow->annotate.cm_alpha_num = (int)((jconf->searchnow->annotate.cm_alpha_end - jconf->searchnow->annotate.cm_alpha_bgn) / jconf->searchnow->annotate.cm_alpha_step) + 1;
      if (jconf->searchnow->annotate.cm_alpha_num > 100) {
	jlog("ERROR: m_option: cm_alpha step num exceeds limit (100)\n");
	return FALSE;
      }
#else
      GET_TMPARG;
      jconf->searchnow->annotate.cm_alpha = (LOGPROB)atof(tmparg);
#endif
      continue;
#ifdef CM_SEARCH_LIMIT
    } else if (strmatch(argv[i],"-cmthres")) { /* CM cut threshold for CM decoding */
      GET_TMPARG;
      jconf->searchnow->annotate.cm_cut_thres = (LOGPROB)atof(tmparg);
      continue;
#endif
#ifdef CM_SEARCH_LIMIT_POP
    } else if (strmatch(argv[i],"-cmthres2")) { /* CM cut threshold for CM decoding */
      GET_TMPARG;
      jconf->searchnow->annotate.cm_cut_thres_pop = (LOGPROB)atof(tmparg);
      continue;
#endif
#endif /* CONFIDENCE_MEASURE */
    } else if (strmatch(argv[i],"-gmm")) { /* load SS parameter from file */
      FREE_MEMORY(jconf->reject.gmm_filename);
      GET_TMPARG;
      jconf->reject.gmm_filename = filepath(tmparg, cwd);
      continue;
    } else if (strmatch(argv[i],"-gmmnum")) { /* num of Gaussian pruning for GMM */
      GET_TMPARG;
      jconf->reject.gmm_gprune_num = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-gmmreject")) {
      GET_TMPARG;
      FREE_MEMORY(jconf->reject.gmm_reject_cmn_string);
      jconf->reject.gmm_reject_cmn_string = strcpy((char *)mymalloc(strlen(tmparg)+1), tmparg);
      continue;
#ifdef GMM_VAD
    } else if (strmatch(argv[i],"-gmmmargin")) { /* backstep margin */
      GET_TMPARG;
      jconf->detect.gmm_margin = atoi(tmparg);
      continue;
    } else if (strmatch(argv[i],"-gmmup")) { /* uptrigger threshold */
      GET_TMPARG;
      jconf->detect.gmm_uptrigger_thres = atof(tmparg);
      continue;
    } else if (strmatch(argv[i],"-gmmdown")) { /* uptrigger threshold */
      GET_TMPARG;
      jconf->detect.gmm_downtrigger_thres = atof(tmparg);
      continue;
#endif
    } else if (strmatch(argv[i],"-htkconf")) {
      GET_TMPARG;
      if (htk_config_file_parse(tmparg, &(jconf->amnow->analysis.para_htk)) == FALSE) {
	jlog("ERROR: m_options: failed to read %s\n", tmparg);
	return FALSE;
      }
      continue;
    } else if (strmatch(argv[i], "-wlist")) {
      GET_TMPARG;
      tmparg = filepath(tmparg, cwd);
      if (multigram_add_prefix_filelist(tmparg, jconf->lmnow, LM_DFA_WORD) == FALSE) {
	jlog("ERROR: m_options: failed to read some word lists\n");
	free(tmparg);
	return FALSE;
      }
      free(tmparg);
      continue;
    } else if (strmatch(argv[i], "-wsil")) {
      /* 
       * if (jconf->lmnow->lmvar != LM_UNDEF && jconf->lmnow->lmvar != LM_DFA_WORD) {
       *   jlog("ERROR: \"-wsil\" only valid for isolated word recognition mode\n");
       *   return FALSE;
       * }
       */
      GET_TMPARG;
      strncpy(jconf->lmnow->wordrecog_head_silence_model_name, tmparg, MAX_HMMNAME_LEN);
      GET_TMPARG;
      strncpy(jconf->lmnow->wordrecog_tail_silence_model_name, tmparg, MAX_HMMNAME_LEN);
      GET_TMPARG;
      if (strmatch(tmparg, "NULL")) {
	jconf->lmnow->wordrecog_silence_context_name[0] = '\0';
      } else {
	strncpy(jconf->lmnow->wordrecog_silence_context_name, tmparg, MAX_HMMNAME_LEN);
      }
      continue;
#ifdef DETERMINE
    } else if (strmatch(argv[i], "-wed")) {
      //if (jconf->lmnow->lmvar != LM_UNDEF && jconf->lmnow->lmvar != LM_DFA_WORD) {
      //jlog("ERROR: \"-wed\" only valid for isolated word recognition mode\n");
      //return FALSE;
      //}
      GET_TMPARG;
      jconf->searchnow->pass1.determine_score_thres = atof(tmparg);
      GET_TMPARG;
      jconf->searchnow->pass1.determine_duration_thres = atoi(tmparg);
      continue;
#endif
    } else if (strmatch(argv[i], "-inactive")) { /* start inactive */
      jconf->searchnow->sw.start_inactive = TRUE;
      continue;
    } else if (strmatch(argv[i], "-active")) { /* start active (default) */
      jconf->searchnow->sw.start_inactive = FALSE;
      continue;
    } else if (strmatch(argv[i],"-fallback1pass")) { /* use 1st pass result on search failure */
      jconf->searchnow->sw.fallback_pass1_flag = TRUE;
      continue;
    }
    if (argv[i][0] == '-' && strlen(argv[i]) == 2) {
      /* 1-letter options */
      switch(argv[i][1]) {
      case 'h':			/* hmmdefs */
	FREE_MEMORY(jconf->amnow->hmmfilename);
	GET_TMPARG;
	jconf->amnow->hmmfilename = filepath(tmparg, cwd);
	break;
      case 'v':			/* dictionary */
	FREE_MEMORY(jconf->lmnow->dictfilename);
	GET_TMPARG;
	jconf->lmnow->dictfilename = filepath(tmparg, cwd);
	break;
      case 'w':			/* word list (isolated word recognition) */
	GET_TMPARG;
	if (multigram_add_prefix_list(tmparg, cwd, jconf->lmnow, LM_DFA_WORD) == FALSE) {
	  jlog("ERROR: m_options: failed to read some word list\n");
	  return FALSE;
	}
	break;
      case 'd':			/* binary N-gram */
	/* lmvar should be overriden by the content of the binary N-gram */
	FREE_MEMORY(jconf->lmnow->ngram_filename);
	FREE_MEMORY(jconf->lmnow->ngram_filename_lr_arpa);
	FREE_MEMORY(jconf->lmnow->ngram_filename_rl_arpa);
	GET_TMPARG;
	jconf->lmnow->ngram_filename = filepath(tmparg, cwd);
	break;
      case 'b':			/* beam width in 1st pass */
	GET_TMPARG;
	jconf->searchnow->pass1.specified_trellis_beam_width = atoi(tmparg);
	break;
      case 's':			/* stack size in 2nd pass */
	GET_TMPARG;
	jconf->searchnow->pass2.stack_size = atoi(tmparg);
	break;
      case 'n':			/* N-best search */
	GET_TMPARG;
	jconf->searchnow->pass2.nbest = atoi(tmparg);
	break;
      case 'm':			/* upper limit of hypothesis generation */
	GET_TMPARG;
	jconf->searchnow->pass2.hypo_overflow = atoi(tmparg);
	break;
      default:
	//jlog("ERROR: m_options: wrong argument: %s\n", argv[0], argv[i]);
	//return FALSE;
	unknown_opt = TRUE;
      }
    } else {			/* error */
      //jlog("ERROR: m_options: wrong argument: %s\n", argv[0], argv[i]);
      //return FALSE;
      unknown_opt = TRUE;
    }
    if (unknown_opt) {
      /* call user-side option processing */
      switch(useropt_exec(jconf, argv, argc, &i)) {
      case 0:			/* does not match user-side options */
	jlog("ERROR: m_options: wrong argument: \"%s\"\n", argv[i]);
	return FALSE;
      case -1:			/* Error in user-side function */
	jlog("ERROR: m_options: error in processing \"%s\"\n", argv[i]);
	return FALSE;
      }
    }
  }
  
  /* set default values if not specified yet */
  for(atmp=jconf->am_root;atmp;atmp=atmp->next) {
    if (!atmp->spmodel_name) {
      atmp->spmodel_name = strcpy((char*)mymalloc(strlen(SPMODEL_NAME_DEFAULT)+1),
			  SPMODEL_NAME_DEFAULT);
    }
  }
  for(ltmp=jconf->lm_root;ltmp;ltmp=ltmp->next) {
    if (!ltmp->head_silname) {
      ltmp->head_silname = strcpy((char*)mymalloc(strlen(BEGIN_WORD_DEFAULT)+1),
				  BEGIN_WORD_DEFAULT);
    }
    if (!ltmp->tail_silname) {
      ltmp->tail_silname = strcpy((char*)mymalloc(strlen(END_WORD_DEFAULT)+1),
				  END_WORD_DEFAULT);
    }
    if (!ltmp->iwspentry) {
      ltmp->iwspentry = strcpy((char*)mymalloc(strlen(IWSPENTRY_DEFAULT)+1),
			       IWSPENTRY_DEFAULT);
    }
  }
#ifdef USE_NETAUDIO
  if (!jconf->input.netaudio_devname) {
    jconf->input.netaudio_devname = strcpy((char*)mymalloc(strlen(NETAUDIO_DEVNAME)+1),
			      NETAUDIO_DEVNAME);
  }
#endif	/* USE_NETAUDIO */

  return TRUE;
}

/** 
 * <JA>
 * ���ץ�����Ϣ�Υ����ΰ���������. 
 * </JA>
 * <EN>
 * Free memories of variables allocated by option arguments.
 * </EN>
 *
 * @param jconf [i/o] jconf configuration data
 *
 * @callgraph
 * @callergraph
 */
void
opt_release(Jconf *jconf)
{
  JCONF_AM *am;
  JCONF_LM *lm;
  JCONF_SEARCH *s;

  FREE_MEMORY(jconf->input.inputlist_filename);
#ifdef USE_NETAUDIO
  FREE_MEMORY(jconf->input.netaudio_devname);
#endif	/* USE_NETAUDIO */
  FREE_MEMORY(jconf->reject.gmm_filename);
  FREE_MEMORY(jconf->reject.gmm_reject_cmn_string);

  for(am=jconf->am_root;am;am=am->next) {
    FREE_MEMORY(am->hmmfilename);
    FREE_MEMORY(am->mapfilename);
    FREE_MEMORY(am->spmodel_name);
    FREE_MEMORY(am->hmm_gs_filename);
    FREE_MEMORY(am->analysis.cmnload_filename);
    FREE_MEMORY(am->analysis.cmnsave_filename);
    FREE_MEMORY(am->frontend.ssload_filename);
  }
  for(lm=jconf->lm_root;lm;lm=lm->next) {
    FREE_MEMORY(lm->ngram_filename);
    FREE_MEMORY(lm->ngram_filename_lr_arpa);
    FREE_MEMORY(lm->ngram_filename_rl_arpa);
    FREE_MEMORY(lm->dfa_filename);
    FREE_MEMORY(lm->head_silname);
    FREE_MEMORY(lm->tail_silname);
    FREE_MEMORY(lm->iwspentry);
    FREE_MEMORY(lm->dictfilename);
    multigram_remove_gramlist(lm);
  }
  for(s=jconf->search_root;s;s=s->next) {
    FREE_MEMORY(s->successive.pausemodelname);
  }
}

/* end of file */
