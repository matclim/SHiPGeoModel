#pragma once

struct ParsedID {
  int type   = -1; // volume type code (wideH, wideV, thinH, thinV, fibreH, fibreV)
  int section= -1; // 0=ecal, 1=hcal
  int layer  = -1; // layer index within that section/type scheme
  int vol    = -1; // bar index or fibre index
  int hcal   = -1;
  int hpl_sublayer = -1;
  int hexant = -1;
};

enum VolumeType : int {
  kWideH  = 1,
  kWideV  = 2,
  kThinH  = 3,
  kThinV  = 4,
  kFibreH = 5,
  kFibreV = 6
};

enum SectionType : int { kEcal=0, kHcal=1 };
