/*
  tarino: a tar implementation with Node.js.
  Copyright 2016 Sam Saint-Pettersen.

  Native add-on.

  Dual licensed under the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

#ifndef __TARINO
#define __TARINO

int write_tar_entry(std::string, std::string, int, int, int);
int write_tar_entries(std::string, std::string);
int extract_tar_entries(std::string, int, int, int);

#endif
