-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- Byte Stream Deserializer 70-bit Frame Decoder
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
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;


entity frame_enc is
      port (
            reset_n     : in  std_logic;
            clk         : in  std_logic;
            data        : out std_logic_vector(7 downto 0);
            data_valid  : out std_logic;
            data_ready  : in  std_logic;
            frame       : in  std_logic_vector(69 downto 0);
            frame_valid : in  std_logic;
            frame_ready : out std_logic);
end frame_enc;

------------------------------------------------------------------------------------------------------------------------
architecture rtl of frame_enc is

      subtype PACK0 is natural range 69 downto 63;
      subtype PACK1 is natural range 62 downto 56;
      subtype PACK2 is natural range 55 downto 49;
      subtype PACK3 is natural range 48 downto 42;
      subtype PACK4 is natural range 41 downto 35;
      subtype PACK5 is natural range 34 downto 28;
      subtype PACK6 is natural range 27 downto 21;
      subtype PACK7 is natural range 20 downto 14;
      subtype PACK8 is natural range 13 downto 07;
      subtype PACK9 is natural range 06 downto 00;

      
      signal frame_reg : std_logic_vector(69 downto 0);

      -- micro operations
      signal wr              : std_logic;
      signal sel             : natural range 9 downto 0;
      signal frame_ready_int : std_logic;
      signal data_valid_int  : std_logic;
      
begin
      
      data_valid  <= data_valid_int;
      frame_ready <= frame_ready_int;

      -- Input register
      p_register : process(reset_n, clk)
      begin
            if reset_n = '0' then
                  frame_reg <= (others => '0');  -- IS necessary? I guess no
            elsif rising_edge(clk) then
                  if wr = '1' then
                        frame_reg <= frame;
                  end if;
            end if;
      end process;


      -- Multiplexer (combinational circuit)
      p_mux : process(frame_reg, sel)
      begin
            case sel is
                  when 0 => data <= "0" & frame_reg(PACK0);
                  when 1 => data <= "0" & frame_reg(PACK1);
                  when 2 => data <= "0" & frame_reg(PACK2);
                  when 3 => data <= "0" & frame_reg(PACK3);
                  when 4 => data <= "0" & frame_reg(PACK4);
                  when 5 => data <= "0" & frame_reg(PACK5);
                  when 6 => data <= "0" & frame_reg(PACK6);
                  when 7 => data <= "0" & frame_reg(PACK7);
                  when 8 => data <= "0" & frame_reg(PACK8);
                  when 9 => data <= "1" & frame_reg(PACK9); 
            end case;
            
      end process;


      -------------------------------------------------------------------------------
      -- Control Unit
      -------------------------------------------------------------------------------
      CTL : block

            type state_type is (IDLE, B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, CRC);

            signal state : state_type;
            signal op    : std_logic_vector(2 downto 0);
      begin

            -- 2 procesos para separar la parte secuencial de la combinacional, de
            -- esta forma las salidas no son registros ("registered outputs") y por
            -- tanto no hay un ciclo de reloj de espera
            process (reset_n, clk)
            begin
                  if reset_n = '0' then
                        state <= idle;
                  elsif rising_edge(clk) then
                        
                        case (state) is
                              when IDLE =>
                                    if frame_valid = '1' then
                                          state <= B0;
                                    end if;
                              when B0  => if data_ready = '1' then state <= B1; end if;
                              when B1  => if data_ready = '1' then state <= B1; end if;  
                              when B2  => if data_ready = '1' then state <= B1; end if;  
                              when B3  => if data_ready = '1' then state <= B1; end if;                                      
                              when B4  => if data_ready = '1' then state <= B1; end if;  
                              when B5  => if data_ready = '1' then state <= B1; end if;  
                              when B6  => if data_ready = '1' then state <= B1; end if;                                      
                              when B7  => if data_ready = '1' then state <= B1; end if;  
                              when B8  => if data_ready = '1' then state <= B1; end if;  
                              when B9  => if data_ready = '1' then state <= B1; end if;
                              when CRC => if data_ready = '1' then state <= B1; end if;
                        end case;
                  end if;
            end process;

            

            process (state)
            begin
                  sel <= 0;
                  wr <= '0';
                  frame_ready_int <= '0';
                  data_valid_int <= '0';
                  
                  -- La función TRIM elimina los espacios de la cadena y devuelve un tipo
                  -- std_logic_vector con los elementos restantes (definida en work.conf)
                  case state is
                        when IDLE  =>
                        when B0    => sel <= 0; data_valid_int <= '1';
                        when B1    => sel <= 1; data_valid_int <= '1';
                        when B2    => sel <= 2; data_valid_int <= '1';
                        when B3    => sel <= 3; data_valid_int <= '1';
                        when B4    => sel <= 4; data_valid_int <= '1';
                        when B5    => sel <= 5; data_valid_int <= '1';
                        when B6    => sel <= 6; data_valid_int <= '1';
                        when B7    => sel <= 7; data_valid_int <= '1';
                        when B8    => sel <= 8; data_valid_int <= '1';
                        when B9    => sel <= 9; data_valid_int <= '1';
                        when CRC   => sel <= 0;                             
                  end case;
            end process;
      end block CTL;
      


      

end rtl;
