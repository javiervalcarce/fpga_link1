LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.numeric_std.all;


entity apb_master_tb is
end apb_master_tb;


architecture sim of apb_master_tb is


      constant SYSTEM_TCLK          : time := 0020.0 ns;  -- @50MHz
      shared variable END_SIMULATION : boolean := false;
        
        
        constant CODE_IDLE         : natural := 16#1#;
  constant CODE_READ32       : natural := 16#2#;
  constant CODE_WRITE32      : natural := 16#3#;
  constant CODE_IDLE_ACK     : natural := 16#8#;
  constant CODE_READ32_ACK   : natural := 16#9#;
  constant CODE_READ32_NACK  : natural := 16#a#;
  constant CODE_WRITE32_ACK  : natural := 16#b#;
  constant CODE_WRITE32_NACK : natural := 16#c#;
  constant CODE_INTERRUPT    : natural := 16#f#;
  
      signal reset_n      : std_logic;
      signal clk          : std_logic;
      
      -- Trama recibida por el maestro APB a través del puerto serie
      signal rx_data   : std_logic_vector(61 downto 00);
      signal rx_valid  : std_logic;
      signal rx_ready  : std_logic;
      
      -- Trama enviada por el maestro APB a través del puerto serie
      signal tx_data   : std_logic_vector(61 downto 00);
      signal tx_valid  : std_logic;
      signal tx_ready  : std_logic;


      signal psel         :     std_logic;
      signal pwrite       :     std_logic;
      signal penable      :     std_logic;
      signal paddr        :     std_logic_vector(23 DOWNTO 00);  
      signal pwdata       :     std_logic_vector(31 DOWNTO 00);
      signal prdata       :      std_logic_vector(31 DOWNTO 00);
      signal pready       :      std_logic;
      
      signal slave_store  :     std_logic_vector(31 DOWNTO 00);
begin

      -- system clock (@50MHz)
      p_clk : process
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

  p_reset : process 
    begin
      reset_n <= '0';
      wait for 4.0 * SYSTEM_TCLK;      
      reset_n <= '1'; 
      wait;
    end process;
      

      p_input: process
      begin
            
            rx_data(61 downto 56) <= std_logic_vector(to_unsigned(0, 6));
            rx_data(55 downto 32) <= X"00_00_00";
            rx_data(31 downto 00) <= X"00_00_00_00";
            
            rx_valid <= '0';
      
            wait until rising_edge(reset_n);
            wait until rising_edge(clk);

            --assert (rx_ready = '1') report "rx_ready <> 1" severity error;
            assert (tx_valid = '0') report "tx_valid <> 0" severity error;

            wait until rising_edge(clk);
            
            -- Primera trama decodificada (sin el CRC de 8 bits).
            rx_valid <= '1';
            rx_data(61 downto 56) <= std_logic_vector(to_unsigned(CODE_WRITE32, 6));
            rx_data(55 downto 32) <= X"01_02_03";
            rx_data(31 downto 00) <= X"aa_bb_cc_dd";
            
            wait until rising_edge(clk) and rx_ready = '1';
            
            
            -- Primera trama decodificada (sin el CRC de 8 bits).
            rx_valid <= '1';
            rx_data(61 downto 56) <= std_logic_vector(to_unsigned(CODE_READ32, 6));
            rx_data(55 downto 32) <= X"04_05_06";
            rx_data(31 downto 00) <= (others => '0');
            
            wait until rising_edge(clk) and rx_ready = '1';
            
            
            
            
            
      end process p_input;
    
    
    
    
  dut : entity work.apb_master port map (
    clk       => clk,
      reset_n  => reset_n,
      -- rx frame
      frame_dec_data  => rx_data,
      frame_dec_valid => rx_valid,
      frame_dec_ready => rx_ready,
      -- tx frame
      frame_enc_data  => tx_data,
      frame_enc_valid => tx_valid,
      frame_enc_ready => tx_ready,
      -- APB Slaves 
      psel         => psel,
      pwrite       => pwrite,
      penable      => penable,
      paddr        => paddr,
      pwdata       => pwdata,
      prdata       => prdata,
      pready       => pready
  );    
  
  
  -- Este proceso es un modelo de periférico APB (no sintetizable)
  p_apbslave : process(reset_n, clk)
    variable c : natural := 0;
  begin
    if reset_n = '0' then
    pready <= '0';
    prdata <= (others => '0');
    slave_store <= (others => '0');
  elsif rising_edge(clk) then
    
    pready <= '0';
    
    if psel = '1' then
      if penable = '0' then
        -- setup phase
      else
        -- access phase, no wait states.
        pready <= '1';
        c := c + 1;
        if pwrite = '0' then
          assert paddr = X"01_02_03";
          
          prdata <= std_logic_vector(to_unsigned(c, 32));  -- read
        else 
          
          assert paddr = X"01_02_03";
          assert pwdata = X"aa_bb_cc_dd";
          -- write operation
          slave_store <= pwdata;
        end if;
      end if;
    end if;
  end if;
  end process p_apbslave;
    
end sim;
