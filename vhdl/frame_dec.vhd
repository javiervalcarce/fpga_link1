-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Byte Stream Serialized N-bit Frame Decoder
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

entity frame_dec is
      generic (
            N : integer := 10);
      port (
            reset_n     : in  std_logic;
            clk         : in  std_logic;
            data        : in  std_logic_vector(7 downto 0);
            data_valid  : in  std_logic;
            data_ready  : out std_logic;
            frame       : out std_logic_vector(69 downto 0);
            frame_valid : out std_logic;
            frame_ready : in  std_logic);
end frame_dec;

------------------------------------------------------------------------------------------------------------------------
architecture rtl of frame_dec is

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
      
      p_position : process(reset_n, clk)
      begin
            if reset_n = '0' then
                  c <= 0;
                  frame_int <= (others => '0');  -- not strictly required
                  frame_valid_int <= '0';
                  
            elsif rising_edge(clk) then
                  
                  if frame_valid_int = '0' and data_valid = '1' then
                        
                        frame_valid_int <= '0';
                        
                        if data(7) = '1' then
                              -- It is the first byte
                              c                <= 1;
                              frame_int(PACK0) <= data(6 downto 0);
                        else
                              
                              if c = 1 then
                                    frame_int(PACK1) <= data(6 downto 0);
                                    c                <= 2;
                              elsif c = 2 then
                                    frame_int(PACK2) <= data(6 downto 0);
                                    c                <= 3;
                              elsif c = 3 then
                                    frame_int(PACK3) <= data(6 downto 0);
                                    c                <= 4;
                              elsif c = 4 then
                                    frame_int(PACK4) <= data(6 downto 0);
                                    c                <= 5;
                              elsif c = 5 then
                                    frame_int(PACK5) <= data(6 downto 0);
                                    c                <= 6;
                              elsif c = 6 then
                                    frame_int(PACK6) <= data(6 downto 0);
                                    c                <= 7;
                              elsif c = 7 then
                                    frame_int(PACK7) <= data(6 downto 0);
                                    c                <= 8;
                              elsif c = 8 then
                                    frame_int(PACK8) <= data(6 downto 0);
                                    c                <= 9;
                              elsif c = 9 then
                                    frame_int(PACK9) <= data(6 downto 0);
                                    c                <= 0;     
                                    -- there is no next state
                                    frame_valid_int  <= '1';
                              end if;


                        end if;

                  end if;

            end if;

      end process;


end rtl;
