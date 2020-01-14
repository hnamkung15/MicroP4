/*
 * Author: Hardik Soni
 * Email: hks57@cornell.edu
 */

#include"msa.p4"
#include"common.p4"

struct l3_meta_t { }

header ipv4_h {
  bit<4> version;
  bit<4> ihl;
  bit<8> diffserv;
  bit<16> totalLen;
  bit<16> identification;
  bit<3> flags;
  bit<13> fragOffset;
  bit<8> ttl;
  bit<8> protocol;
  bit<16> hdrChecksum;
  bit<32> srcAddr;
  bit<32> dstAddr; 
}

struct l3v4_hdr_t {
  ipv4_h ipv4;
}

cpackage L3v4 : implements Unicast<l3v4_hdr_t, l3_meta_t, empty_t, bit<16>, empty_t> {

  parser micro_parser(extractor ex, pkt p, im_t im, out l3v4_hdr_t hdr, 
                      inout l3_meta_t meta, in empty_t ia, inout empty_t ioa) {
    state start {
      ex.extract(p, hdr.ipv4);
      transition accept;
    }
  }

  control micro_control(pkt p, im_t im, inout l3v4_hdr_t hdr, inout l3_meta_t m,
                          in empty_t e, out bit<16> nexthop, 
                          inout empty_t ioa) { // nexthop out arg
    action process(bit<16> nh) {
      hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
      nexthop = nh;  // setting out param
    }
    action default_act() {
      nexthop = 0; 
    }

    table ipv4_lpm_tbl {
      key = { 
        hdr.ipv4.dstAddr : lpm; 
      } 
      actions = { 
        process; 
        default_act;
      }
      default_action = default_act;

    }
    apply { 
      ipv4_lpm_tbl.apply(); 
    }
  }

  control micro_deparser(emitter em, pkt p, in l3v4_hdr_t h) {
    apply { 
      em.emit(p, h.ipv4); 
    }
  }
}

 
