-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Serial Line IP decoder.
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

-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with this program. If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;



entity slip_dec_tb is
end slip_dec_tb;

------------------------------------------------------------------------------------------------------------------------

architecture sim of slip_dec_tb is
      
      signal reset            : std_logic;
      signal clk            : std_logic;
      signal data_o            : std_logic_vector(7 downto 0);
      signal data_i            : std_logic_vector(7 downto 0);
      signal valid : std_logic;
      
      --attribute INIT : string; 
      --attribute INIT of mux1_lut      : label is "E4FF";

      component slip_dec
            port (
                  reset   : in    std_logic;
                  clk     : in    std_logic;
                  data_i  : in    std_logic_vector(7 downto 0);
                  data_o  : out   std_logic_vector(7 downto 0);
                  valid   : out   std_logic;       
                  ready   : out   std_logic);
      end component;
      
begin
      
      uut : slip_dec port map (
            reset   => reset,
            clk     => clk,
            data_i  => data_i,
            data_o  => data_o,
            valid   => valid,
            ready   => valid
      );
            

end sim;
