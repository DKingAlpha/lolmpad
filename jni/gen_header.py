
import os
import re
import sys
input_prop_list = []
ev_list = []
syn_list = []
key_list = []
rel_list = []
abs_list = []
sw_list = []
msc_list = []
led_list = []
rep_list = []
snd_list = []
mt_tool_list = []
ff_status_list = []
ff_list = []
r = re.compile(r'#define\s+(\S+)\s+((?:0x)?\d+)')
for arg in sys.argv[1:]:
  with open(arg, 'r') as f:
    for line in f:
      m = r.match(line)
      if m:
        name = m.group(1)
        if name.startswith("INPUT_PROP_"):
          input_prop_list.append(name)
        elif name.startswith("EV_"):
          ev_list.append(name)
        elif name.startswith("SYN_"):
          syn_list.append(name)
        elif name.startswith("KEY_") or name.startswith("BTN_"):
          key_list.append(name)
        elif name.startswith("REL_"):
          rel_list.append(name)
        elif name.startswith("ABS_"):
          abs_list.append(name)
        elif name.startswith("SW_"):
          sw_list.append(name)
        elif name.startswith("MSC_"):
          msc_list.append(name)
        elif name.startswith("LED_"):
          led_list.append(name)
        elif name.startswith("REP_"):
          rep_list.append(name)
        elif name.startswith("SND_"):
          snd_list.append(name)
        elif name.startswith("MT_TOOL_"):
          mt_tool_list.append(name)
        elif name.startswith("FF_STATUS_"):
          ff_status_list.append(name)
        elif name.startswith("FF_"):
          ff_list.append(name)
def Dump(struct_name, values):
  print('static struct label %s[] = {' % (struct_name))
  for value in values:
    print('    LABEL(%s),' % (value))
  print('    LABEL_END,')
  print('};')
Dump("input_prop_labels", input_prop_list)
Dump("ev_labels", ev_list)
Dump("syn_labels", syn_list)
Dump("key_labels", key_list)
Dump("rel_labels", rel_list)
Dump("abs_labels", abs_list)
Dump("sw_labels", sw_list)
Dump("msc_labels", msc_list)
Dump("led_labels", led_list)
Dump("rep_labels", rep_list)
Dump("snd_labels", snd_list)
Dump("mt_tool_labels", mt_tool_list)
Dump("ff_status_labels", ff_status_list)
Dump("ff_labels", ff_list)
