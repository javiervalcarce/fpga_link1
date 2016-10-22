LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;

entity apb_master_tb is
end apb_master_tb;


architecture sim of apb_master_tb is
      constant SYSTEM_TCLK          : time := 0020.0 ns;  -- @50MHz
      
      
      shared variable END_SIMULATION : boolean := false;
      
      signal reset_n      : std_logic;
      signal clk          : std_logic;
      
      signal dec_data   : std_logic_vector(61 downto 00);
      signal dec_valid  : std_logic;
      signal dec_ready  : std_logic;
      signal enc_data   : std_logic_vector(61 downto 00);
      signal enc_valid  : std_logic;
      signal enc_ready  : std_logic;

      
      --attribute INIT : string; 
      --attribute INIT of mux1_lut : label is "E4FF";
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

      p_input: process
          
      begin

            dec_data  <= B"000001" & X"00_00_00_00_00_00_00";
            dec_valid <= '0';
      
            wait until rising_edge(reset_n);

            assert (dec_ready = '1') report "dec_ready <> 1" severity error;
            assert (enc_valid = '0') report "enc_valid <> 0" severity error;

            wait until rising_edge(clk);
            
            -- Primera trama decodificada (sin el CRC de 8 bits).
            dec_valid <= '1';
            dec_data <= B"000001" & X"00_00_00_aa_bb_cc_dd";
            
            wait until rising_edge(clk) and dec_ready = '1';
            

            wait;
            
      end process p_input;
    
    
end sim;
