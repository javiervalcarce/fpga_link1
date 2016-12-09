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
entity crc8d8_tb is
end crc8d8_tb;

------------------------------------------------------------------------------------------------------------------------
architecture sim of crc8d8_tb is

      constant SYSTEM_TCLK  : time    := 0020.0 ns;  -- @50MHz
      signal END_SIMULATION : boolean := false;

      signal reset_n : std_logic;
      signal clk     : std_logic;
      signal data    : std_logic_vector(07 downto 00);
      signal en      : std_logic;
      signal rs      : std_logic;
      signal crc     : std_logic_vector(07 downto 00);

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

            data <= (others => '1');
            rs <= '0';
            en <= '0';

            wait until rising_edge(reset_n);

            rs <= '1';
            en <= '0';
            wait until rising_edge(clk);
            rs <= '0';
            en <= '1';


            data <= X"f1";
            wait until rising_edge(clk);

            data <= X"f2";
            wait until rising_edge(clk);

            data <= X"a1";
            wait until rising_edge(clk);

            data <= X"ee";
            wait until rising_edge(clk);

            data <= X"03";
            wait until rising_edge(clk);

            data <= X"9e";
            wait until rising_edge(clk);

            data <= X"47";
            wait until rising_edge(clk);

            data <= X"ff";
            wait until rising_edge(clk);

            en <= '0';
            wait until rising_edge(clk);
            wait until rising_edge(clk);

            rs <= '1';
            en <= '0';
            wait until rising_edge(clk);

            
            END_SIMULATION <= true;
      end process;

      -- entity instantiation (VHDL'93)
      uut : entity work.crc8d8 port map (
            clk     => clk,
            --
            rs      => rs,
            en      => en,
            data    => data,
            --
            crc     => crc
            );

end sim;
