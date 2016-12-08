-- Hi Emacs, this is -*- mode: vhdl; vhdl-basic-offset: 6 -*-
------------------------------------------------------------------------------------------------------------------------
--
-- TOP LEVEL FPGA DESIGN UNIT
--
-- 
--
-- Copyright (c) 2016 Javier Valcarce GarcÃ­a, <javier.valcarce@gmail.com>
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
use IEEE.numeric_std.all;

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
            external_reset : in  std_logic;
            clk            : in  std_logic;
            serial_tx      : out std_logic;
            serial_rx      : in  std_logic;
            if7seg_an      : out std_logic_vector(3 downto 0);
            if7seg_do      : out std_logic_vector(7 downto 0);
            led            : out std_logic_vector(7 downto 0));
end top;



------------------------------------------------------------------------------------------------------------------------
architecture rtl of top is

      -- Puerto serie. Generación de pulsos en16
      signal en16    : std_logic;
      signal counter : unsigned(15 downto 0);

      signal reset    : std_logic;
      signal reset_n  : std_logic;
      signal clk_slow : std_logic;

      signal read_buffer  : std_logic;
      signal reset_buffer : std_logic;

      -- Avalon-ST signals for serial port TX
      signal tx_data  : std_logic_vector(7 downto 0);  -- bytes que se envían                                                      -- tx
      signal tx_valid : std_logic;
      signal tx_ready : std_logic;
      signal tx_full  : std_logic;

      -- Avalon-ST signals for serial port TX
      signal rx_data  : std_logic_vector(7 downto 0);  -- bytes que se reciben
      signal rx_valid : std_logic;
      signal rx_ready : std_logic;

      signal frame_dec_data  : std_logic_vector(61 downto 00);
      signal frame_dec_valid : std_logic;
      signal frame_dec_ready : std_logic;


      signal frame_enc_data  : std_logic_vector(61 downto 00);
      signal frame_enc_valid : std_logic;
      signal frame_enc_ready : std_logic;

      --apb signals

      signal psel    : std_logic;
      signal pwrite  : std_logic;
      signal penable : std_logic;
      signal paddr   : std_logic_vector(23 downto 00);
      signal pwdata  : std_logic_vector(31 downto 00);
      signal prdata  : std_logic_vector(31 downto 00);
      signal pready  : std_logic;

      constant CLK_FREQUENCY     : natural := 50_000_000;
      constant SERIAL_PORT_SPEED : natural := 9600;

begin

      counter  <= counter + 1 when rising_edge(clk);
      clk_slow <= counter(15);  -- 50 MHz / 2^16 = 763Hz for debouncing flip-flops

      -- Debouncer circuit for external reset signal
      reset    <= external_reset when rising_edge(clk_slow);
      reset_n  <= not reset;
      tx_ready <= not tx_full;


      led <= frame_dec_data(31 downto 24);

      div : process(clk, reset_n)
            -- ¿Cuántos periodos de reloj externo |clk| hay en un periodo de
            -- bit. Pues Fclk / bps. Así pues hay que generar un pulso de en16
            -- cada Fclk / bps / 16
            --
            -- Fclk = 50 MHz, bps = 9600 => Cada 50e6/9600/16 = 326 con un
            -- error relativo del 0,15 %
            variable c : natural range 0 to CLK_FREQUENCY/SERIAL_PORT_SPEED;  --326;
      begin
            if reset_n = '0' then
                  en16 <= '0';
            elsif rising_edge(clk) then
                  c    := c + 1;
                  en16 <= '0';
                  if c = 325 then
                        c    := 0;
                        en16 <= '1';
                  end if;
            end if;
      end process;



      -- Receptor del puerto serie
      urx : entity work.uart_rx port map (
            -- in
            clk                 => clk,
            serial_in           => serial_rx,
            read_buffer         => rx_ready,
            reset_buffer        => reset_buffer,
            en_16_x_baud        => en16,
            -- out
            data_out            => rx_data,
            buffer_data_present => rx_valid,
            buffer_full         => open,
            buffer_half_full    => open);

      -- Transmisor del puerto serie
      utx : entity work.uart_tx port map (
            -- in
            clk              => clk,
            data_in          => tx_data,
            write_buffer     => tx_valid,
            reset_buffer     => reset_buffer,
            en_16_x_baud     => en16,
            -- out
            serial_out       => serial_tx,
            buffer_full      => tx_full,
            buffer_half_full => open);


      -- Decodificador de tramas
      dec : entity work.frame_dec port map (
            clk         => clk,
            reset_n     => reset_n,
            -- in
            octet_data  => rx_data,
            octet_valid => rx_valid,
            octet_ready => rx_ready,
            -- out
            frame_data  => frame_dec_data,
            frame_valid => frame_dec_valid,
            frame_ready => frame_dec_ready
            );



      enc : entity work.frame_enc port map (
            reset_n     => reset_n,
            clk         => clk,
            -- in
            octet_data  => tx_data,
            octet_valid => tx_valid,
            octet_ready => tx_ready,
            -- out
            frame_data  => frame_enc_data,
            frame_valid => frame_enc_valid,
            frame_ready => frame_enc_ready
            );

      apb : entity work.apb_master port map (

            clk            => clk,
            reset_n        => reset_n,
            -- in
            rx_frame_data  => frame_dec_data,
            rx_frame_valid => frame_dec_valid,
            rx_frame_ready => frame_dec_ready,
            -- out
            tx_frame_data  => frame_enc_data,
            tx_frame_valid => frame_enc_valid,
            tx_frame_ready => frame_enc_ready,
            --APB IF 
            psel           => psel,
            pwrite         => pwrite,
            penable        => penable,
            paddr          => paddr,
            pwdata         => pwdata,
            prdata         => prdata,
            pready         => pready
            );

      seg : entity work.interface_7segx4_apb port map (
            reset_n   => reset_n,
            clk       => clk,
            psel      => psel,
            penable   => penable,
            pwrite    => pwrite,
            paddr     => paddr,
            pwdata    => pwdata,
            prdata_in => X"BA_BB_BC_BD",
            pready_in => '1',
            prdata    => prdata,
            pready    => pready,
            do        => if7seg_do,
            an        => if7seg_an
            );

end rtl;
