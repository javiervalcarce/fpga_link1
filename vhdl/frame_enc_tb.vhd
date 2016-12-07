-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Test Bench for frame_enc.
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
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

------------------------------------------------------------------------------------------------------------------------
entity frame_enc_tb is
end frame_enc_tb;

------------------------------------------------------------------------------------------------------------------------
architecture sim of frame_enc_tb is

      constant SYSTEM_TCLK : time := 0020.0 ns;  -- @50MHz

      signal END_SIMULATION : boolean := false;

      signal reset_n     : std_logic;
      signal clk         : std_logic;
      signal octet_data  : std_logic_vector(07 downto 00);
      signal octet_valid : std_logic;
      signal octet_ready : std_logic;
      signal frame_data  : std_logic_vector(61 downto 00);
      signal frame_valid : std_logic;
      signal frame_ready : std_logic;

      --attribute INIT : string; 
      --attribute INIT of mux1_lut      : label is "E4FF";



begin

      -- system clock (@50MHz)
      p_system_clk : process
      begin
            if END_SIMULATION = false then
                  clk <= '0';
                  wait for SYSTEM_TCLK/2;
                  clk <= '1';
                  wait for SYSTEM_TCLK/2;
            else
                  wait;
            end if;
      end process;

      reset_n <= '0' after 0.0 ns,
                 '1' after 4.0 * SYSTEM_TCLK;

      p_input : process
      begin
            
            frame_data <= "111110" & X"aa_bb_cc_dd_11_22_33";
            frame_valid <= '1';
            octet_ready <= '1';
            
            wait until rising_edge(reset_n);

            -- give some time more to process the last byte
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            wait until rising_edge(clk);
            

            wait until frame_ready = '1';

            END_SIMULATION <= true;
      end process;

      -- entity instantiation (VHDL'93)
      uut : entity work.frame_enc port map (
            reset_n     => reset_n,
            clk         => clk,
            --
            octet_data  => octet_data,
            octet_valid => octet_valid,
            octet_ready => octet_ready,
            --
            frame_data  => frame_data,
            frame_valid => frame_valid,
            frame_ready => frame_ready
            );

end sim;
