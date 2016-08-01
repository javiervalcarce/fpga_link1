-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Edge Detector
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

library ieee;
use ieee.std_logic_1164.all;


entity edge_detector is
      port(
            clk   : in  std_logic;
            rst_n : in  std_logic;
            input : in  std_logic;
            re    : out std_logic;
            fe    : out std_logic
            );
end edge_detector;


------------------------------------------------------------------------------------------------------------------------
architecture rtl of edge_detector is
      signal tmp0, tmp1 : std_logic;
begin
      process (rst_n, clk)
      begin
            if rst_n = '0' then
                  tmp0 <= '0';
                  tmp1 <= '0';
            elsif rising_edge(clk) then
                  tmp1 <= tmp0;
                  tmp0 <= input;
            end if;
            
      end process;

      re <= tmp0 and not(tmp1);
      fe <= tmp1 and not(tmp0);
      
end architecture rtl;
