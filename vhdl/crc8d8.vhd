-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Byte Stream Deserializer 70-bit Frame Decoder
--
-- Created:           2016-08-01
-- Last Modification: 2016-08-01
--
-- Copyright (c) 2016 Javier Valcarce García, <javier.valcarce@gmail.com>
-- http://javiervalcarce.eu
--
------------------------------------------------------------------------------------------------------------------------
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.

-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this program. If not, see <http://www.gnu.org/licenses/>.

------------------------------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

--
-- This entity has an Avalon-ST interface.
-- Ready Latency: zero
--
-- The signals _valid and _ready does not need explanation. See Avalon-ST
-- interface specification.
--
-- This block accepts a 62 bits frame in bus |frame_data| and output a 70 bits
-- (8 bits crc appended) frame byte by byte in bus |octet_data|. 
entity crc8d8 is
      port (
            clk  : in  std_logic;
            --            
            rs   : in  std_logic;       -- synchronous reset
            en   : in  std_logic;
            data : in  std_logic_vector(07 downto 00);
            crc  : out std_logic_vector(07 downto 00));
end crc8d8;


------------------------------------------------------------------------------------------------------------------------
architecture rtl of crc8d8 is

      -- polynomial: x^8 + x^2 + x^1 + 1
      -- data width: 8
      -- convention: the first serial bit is D[7]
      function nextCRC8_D8
            (Data : std_logic_vector(7 downto 0);
             crc  : std_logic_vector(7 downto 0))
            return std_logic_vector is

            variable d      : std_logic_vector(7 downto 0);
            variable c      : std_logic_vector(7 downto 0);
            variable newcrc : std_logic_vector(7 downto 0);

      begin
            d := Data;
            c := crc;

            newcrc(0) := d(7) xor d(6) xor d(0) xor c(0) xor c(6) xor c(7);
            newcrc(1) := d(6) xor d(1) xor d(0) xor c(0) xor c(1) xor c(6);
            newcrc(2) := d(6) xor d(2) xor d(1) xor d(0) xor c(0) xor c(1) xor c(2) xor c(6);
            newcrc(3) := d(7) xor d(3) xor d(2) xor d(1) xor c(1) xor c(2) xor c(3) xor c(7);
            newcrc(4) := d(4) xor d(3) xor d(2) xor c(2) xor c(3) xor c(4);
            newcrc(5) := d(5) xor d(4) xor d(3) xor c(3) xor c(4) xor c(5);
            newcrc(6) := d(6) xor d(5) xor d(4) xor c(4) xor c(5) xor c(6);
            newcrc(7) := d(7) xor d(6) xor d(5) xor c(5) xor c(6) xor c(7);
            return newcrc;
      end nextCRC8_D8;


      signal current : std_logic_vector(07 downto 00);

begin

      crc <= current;

      p_oreg : process(clk)
      begin
            if rising_edge(clk) then
                  if rs = '1' then
                        current <= (others => '0');
                  else
                        if en = '1' then
                              current <= nextCRC8_D8(data, current);
                        end if;
                  end if;
            end if;
      end process;

end rtl;
