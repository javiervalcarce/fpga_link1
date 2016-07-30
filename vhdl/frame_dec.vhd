-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
----------------------------------------------------------------------------------------------------
--
-- Byte Stream Serialized 56-bit Frame Decoder
--
-- 
--
-- Copyright (c) 2016 Javier Valcarce García, <javier.valcarce@gmail.com>
-- http://javiervalcarce.eu
--
----------------------------------------------------------------------------------------------------
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
----------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

------------------------------------------------------------------------------------
--
-- Main Entity for KCUART_TX
--

entity frame56_dec is
      port (
            reset               : in    std_logic;
            clk                 : in    std_logic;
            data_i              : in    std_logic_vector(7 downto 0);
            data_o              : out   std_logic_vector(7 downto 0);
            valid               : out   std_logic;       
            ready               : out   std_logic);
end frame56_dec;

------------------------------------------------------------------------------------
--
-- Start of Main Architecture for KCUART_TX
--	 
architecture rtl of frame56_dec is
--
      signal data_01            : std_logic;
      signal data_23            : std_logic;
      signal data_45            : std_logic;
      --attribute INIT : string; 
      --attribute INIT of mux1_lut      : label is "E4FF";

begin
     


end rtl;
