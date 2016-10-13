-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- TOP LEVEL FPGA DESIGN UNIT
--
-- 
--
-- Copyright (c) 2016 Javier Valcarce García, <javier.valcarce@gmail.com>
-- http://javiervalcarce.eu
--
------------------------------------------------------------------------------------------------------------------------
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this program. If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;

------------------------------------------------------------------------------------------------------------------------
--
-- This block has an interface compatible with Avalon-ST.
--
-- This block process an input stream of encoded bytes, detects frame limits and outputs the decoded frame.
-- Inputs 10 encoded bytes and output a frame of 9 bytes.
--
--
entity top is
      port (
            reset       : in  std_logic;
            clk         : in  std_logic;
            serial_tx   : out  std_logic;
            serial_rx   : in  std_logic;
            if7segx4_an : out std_logic_vector(3 downto 0);
            if7segx4_do : out std_logic_vector(7 downto 0);
            led         : out   std_logic_vector(07 downto 00));       
end top;



------------------------------------------------------------------------------------------------------------------------
architecture rtl of top is

      subtype PACK0 is natural range 69 downto 63;
      subtype PACK1 is natural range 62 downto 56;
      subtype PACK2 is natural range 55 downto 49;
      subtype PACK3 is natural range 48 downto 42;
      subtype PACK4 is natural range 41 downto 35;
      subtype PACK5 is natural range 34 downto 28;
      subtype PACK6 is natural range 27 downto 21;
      subtype PACK7 is natural range 20 downto 14;
      subtype PACK8 is natural range 13 downto 7;
      subtype PACK9 is natural range 6 downto 0;

      signal frame_int        : std_logic_vector(69 downto 0);
      signal frame_valid_int  : std_logic;
      signal c                : integer range 0 to 9;

begin
      frame <= frame_int;
      frame_valid <= frame_valid_int;

      data_ready <= '1'; -- tmp
      


end rtl;
