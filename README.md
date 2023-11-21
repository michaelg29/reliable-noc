# redundant-noc

## TODO

### Redundancy strategy
* Idea of RO ports (only read from port), WO ports (only write to port), RW (read and write port)
  * noc_wdir_e: NOC_WDIR_TILE, NOC_WDIR_XPLUS, NOC_WDIR_XMINUS, NOC_WDIR_YPLUS, NOC_WDIR_YMINUS
  * noc_rdir_e: NOC_RDIR_TILE0, NOC_RDIR_TILE1, NOC_RDIR_TILE2, NOC_RDIR_XPLUS, NOC_RDIR_XMINUS, NOC_RDIR_YPLUS, NOC_RDIR_YMINUS
    * noc_adapter : noc_adapter_if
      * read_packet(dir, ...)
        * if dir == NOC_RDIR_TILE0 || dir == NOC_RDIR_TILE1 || dir == NOC_RDIR_TILE2
* Control bits
  * is_redundant - true if want to send redundant packet, cleared after duplicated
  * redundant_id - map to three x-y coordinates: xy0[7:0], xy1[7:0], xy2[7:0]
