<<<<<<< refs/remotes/upstream/main
-- ----------------------------------------------------------------------------	
=======
-- ----------------------------------------------------------------------------
>>>>>>> Revert "enlever le chain de argu"
-- FILE: 	FT601_arb.vhd
-- DESCRIPTION:	FT601 arbitrator
-- DATE:	May 13, 2016
-- AUTHOR(s):	Lime Microsystems
-- REVISIONS:
<<<<<<< refs/remotes/upstream/main
-- ----------------------------------------------------------------------------	
=======
-- ----------------------------------------------------------------------------
>>>>>>> Revert "enlever le chain de argu"
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- ----------------------------------------------------------------------------
-- Entity declaration
-- ----------------------------------------------------------------------------
entity FT601_arb is
   generic(
<<<<<<< refs/remotes/upstream/main
      FT_data_width     : integer := 32;	
=======
      FT_data_width     : integer := 32;
>>>>>>> Revert "enlever le chain de argu"
      EP82_fifo_rwidth  : integer := 10;
      EP82_wsize        : integer := 64;		--packet size in bytes, has to be multiple of 4 bytes
      EP83_fifo_rwidth  : integer := 10;
      EP83_wsize        : integer := 2048		--packet size in bytes, has to be multiple of 4 bytes
   );
  port (
<<<<<<< refs/remotes/upstream/main
      --input ports 
=======
      --input ports
>>>>>>> Revert "enlever le chain de argu"
      clk               : in std_logic;
      reset_n           : in std_logic;
      enable            : in std_logic;
      --control EP PC->FPGA
      EP02_fifo_data    : out std_logic_vector(FT_data_width-1 downto 0);
      EP02_fifo_wr      : out std_logic;
      EP02_fifo_wrempty : in std_logic;
      --control EP FPGA->PC
      EP82_fifo_data    : in std_logic_vector(FT_data_width-1 downto 0);
      EP82_fifo_rd      : out std_logic;
      EP82_fifo_rdusedw : in std_logic_vector(EP82_fifo_rwidth-1 downto 0);
      --stream EP FPGA->PC
      EP83_fifo_data    : in std_logic_vector(FT_data_width-1 downto 0);
      EP83_fifo_rd      : out std_logic;
      EP83_fifo_rdusedw : in std_logic_vector(EP83_fifo_rwidth-1 downto 0);
      --fsm controll signals
      fsm_epgo          : out std_logic;
      fsm_rdwr          : out std_logic; 		-- 0- MASTER RD (PC->FPGA), 1-MASTER WR (FPGA->PC)
      fsm_ch            : out std_logic_vector(3 downto 0);
      fsm_rdy           : in std_logic;
      fsm_rddata_valid  : in std_logic;
      fsm_rddata        : in std_logic_vector(FT_data_width-1 downto 0);
      fsm_wrdata_req    : in std_logic;
      fsm_wrdata        : out std_logic_vector(FT_data_width-1 downto 0);
<<<<<<< refs/remotes/upstream/main
      
      ep_status         : in std_logic_vector(7 downto 0) -- 0 - EP is ready - Upper nibble: out, lower nibble: in
        
=======

      ep_status         : in std_logic_vector(7 downto 0) -- 0 - EP is ready - Upper nibble: out, lower nibble: in

>>>>>>> Revert "enlever le chain de argu"
        );
end FT601_arb;

-- ----------------------------------------------------------------------------
-- Architecture
-- ----------------------------------------------------------------------------
architecture arch of FT601_arb is

<<<<<<< refs/remotes/upstream/main
type state_type is (idle, check_priority, check_ep02, check_ep82, check_ep83, 
=======
type state_type is (idle, check_priority, check_ep02, check_ep82, check_ep83,
>>>>>>> Revert "enlever le chain de argu"
                    go_ep02, go_ep82, go_ep83);
signal current_state, next_state : state_type;

signal en_ep02, en_ep82, en_ep83 : std_logic;

signal ep_priority	: unsigned(1 downto 0);
signal ep_checked		: std_logic;


<<<<<<< refs/remotes/upstream/main
  
begin
  
=======

begin

>>>>>>> Revert "enlever le chain de argu"
--endpoint ready signals, indicates when transfer can occur
en_ep02<='1' when EP02_fifo_wrempty='1'                                     and ep_status(4)='0'	and fsm_rdy='1' else '0';
en_ep82<='1' when unsigned(EP82_fifo_rdusedw)>=EP82_wsize/(FT_data_width/8) and ep_status(0)='0'	and fsm_rdy='1' else '0';
en_ep83<='1' when unsigned(EP83_fifo_rdusedw)>=EP83_wsize/(FT_data_width/8) and ep_status(1)='0'	and fsm_rdy='1' else '0';

--indicates when endpoint status was checked
ep_checked<='1' when current_state=check_ep02 or current_state=check_ep82 or current_state=check_ep83 else '0';
<<<<<<< refs/remotes/upstream/main
							
--endpoint fifo signals							
=======

--endpoint fifo signals
>>>>>>> Revert "enlever le chain de argu"
EP83_fifo_rd      <= fsm_wrdata_req     when ep_priority=0 else '0';

EP02_fifo_wr      <= fsm_rddata_valid   when ep_priority=1 else '0';
EP02_fifo_data    <= fsm_rddata;

EP82_fifo_rd      <= fsm_wrdata_req     when ep_priority=2 else '0';

<<<<<<< refs/remotes/upstream/main
fsm_wrdata		   <= EP82_fifo_data when ep_priority=2 else 
=======
fsm_wrdata		   <= EP82_fifo_data when ep_priority=2 else
>>>>>>> Revert "enlever le chain de argu"
                     EP83_fifo_data;

-- ----------------------------------------------------------------------------
--Transfer start signal to FTDI FSM
-- ----------------------------------------------------------------------------
process(current_state)
<<<<<<< refs/remotes/upstream/main
  begin 
    if current_state=go_ep02 OR current_state=go_ep82 OR current_state=go_ep83 then 
      fsm_epgo<='1';
    else 
=======
  begin
    if current_state=go_ep02 OR current_state=go_ep82 OR current_state=go_ep83 then
      fsm_epgo<='1';
    else
>>>>>>> Revert "enlever le chain de argu"
      fsm_epgo<='0';
  end if;
end process;

-- ----------------------------------------------------------------------------
--Direction control signal
-- ----------------------------------------------------------------------------
process(current_state)
<<<<<<< refs/remotes/upstream/main
  begin 
    if current_state=go_ep02 then 
      fsm_rdwr<='0';
    else 
=======
  begin
    if current_state=go_ep02 then
      fsm_rdwr<='0';
    else
>>>>>>> Revert "enlever le chain de argu"
      fsm_rdwr<='1';
  end if;
end process;

-- ----------------------------------------------------------------------------
--Channel control signal
-- ----------------------------------------------------------------------------
process(current_state)
<<<<<<< refs/remotes/upstream/main
  begin 
    if current_state=go_ep02 OR current_state=go_ep82 then 
      fsm_ch<=x"1";
    else 
=======
  begin
    if current_state=go_ep02 OR current_state=go_ep82 then
      fsm_ch<=x"1";
    else
>>>>>>> Revert "enlever le chain de argu"
      fsm_ch<=x"2";
  end if;
end process;

-- ----------------------------------------------------------------------------
--Endpoint priority counter
-- ----------------------------------------------------------------------------
process(clk, reset_n)begin
	if(reset_n = '0')then
		ep_priority<=(others=>'0');
	elsif(clk'event and clk = '1')then
<<<<<<< refs/remotes/upstream/main
		if ep_checked='1' then 
=======
		if ep_checked='1' then
>>>>>>> Revert "enlever le chain de argu"
			if ep_priority(1)='1' then -- only three states in RX only mode
				ep_priority<=(others=>'0');
			else
				ep_priority<=ep_priority+1;
			end if;
<<<<<<< refs/remotes/upstream/main
		else 
			ep_priority<=ep_priority;
		end if;	
	end if;	
=======
		else
			ep_priority<=ep_priority;
		end if;
	end if;
>>>>>>> Revert "enlever le chain de argu"
end process;


-- ----------------------------------------------------------------------------
--state machine
-- ----------------------------------------------------------------------------

fsm_f : process(clk, reset_n)begin
	if(reset_n = '0')then
		current_state <= idle;
<<<<<<< refs/remotes/upstream/main
	elsif(clk'event and clk = '1')then 
		current_state <= next_state;
	end if;	
=======
	elsif(clk'event and clk = '1')then
		current_state <= next_state;
	end if;
>>>>>>> Revert "enlever le chain de argu"
end process;

-- ----------------------------------------------------------------------------
--state machine combo
-- ----------------------------------------------------------------------------
fsm : process(current_state, enable, fsm_rdy, en_ep02, en_ep82, en_ep83, ep_priority) begin
	next_state <= current_state;
	case current_state is
<<<<<<< refs/remotes/upstream/main
	  
		when idle => 						--idle state, wait for enable
		  if enable='1' then 
		    next_state<=check_priority;
		  else 
		    next_state<=idle;
		  end if;
		  
		when check_priority =>			-- if FTDI fsm is ready check endpoint status
		  if fsm_rdy='1' then 
		    if  ep_priority=0 then
		      next_state<=check_ep02; 
		    elsif ep_priority=1 then 
		      next_state<=check_ep82; 
		    else 
		      next_state<=check_ep83;	    
		    end if;
		  else 
		    next_state<=check_priority;
		  end if;
		  
		when check_ep02 => 				-- check EP02 status
				if en_ep02='1' then
					next_state<=go_ep02; 
				else 
					next_state<=check_priority;
				end if;
			
		when check_ep82 =>  				-- check EP82 status
				if en_ep82='1' then 
					next_state<=go_ep82;
				else 
					next_state<=check_priority;
				end if;
			
		when check_ep83 => 				-- check EP83 status
				if en_ep83='1' then 
					next_state<=go_ep83;
				else 
					if enable = '1' then 
						next_state<=check_priority;
					else 
						next_state<=idle;
					end if;
				end if;
						
		when go_ep02 =>					--start transfer from EP02
			if fsm_rdy='0' then 
				next_state<=check_priority;
			else 
				next_state<=go_ep02;
			end if;
			
		when go_ep82 =>					--start transfer to EP82
			if fsm_rdy='0' then 
				next_state<=check_priority;
			else 
				next_state<=go_ep82;
			end if;   
			
		when go_ep83 =>					--start transfer from EP83
			if fsm_rdy='0' then 
				next_state<=check_priority;
			else 
				next_state<=go_ep83;
			end if; 
				 
			
		when others => 
=======

		when idle => 						--idle state, wait for enable
		  if enable='1' then
		    next_state<=check_priority;
		  else
		    next_state<=idle;
		  end if;

		when check_priority =>			-- if FTDI fsm is ready check endpoint status
		  if fsm_rdy='1' then
		    if  ep_priority=0 then
		      next_state<=check_ep02;
		    elsif ep_priority=1 then
		      next_state<=check_ep82;
		    else
		      next_state<=check_ep83;
		    end if;
		  else
		    next_state<=check_priority;
		  end if;

		when check_ep02 => 				-- check EP02 status
				if en_ep02='1' then
					next_state<=go_ep02;
				else
					next_state<=check_priority;
				end if;

		when check_ep82 =>  				-- check EP82 status
				if en_ep82='1' then
					next_state<=go_ep82;
				else
					next_state<=check_priority;
				end if;

		when check_ep83 => 				-- check EP83 status
				if en_ep83='1' then
					next_state<=go_ep83;
				else
					if enable = '1' then
						next_state<=check_priority;
					else
						next_state<=idle;
					end if;
				end if;

		when go_ep02 =>					--start transfer from EP02
			if fsm_rdy='0' then
				next_state<=check_priority;
			else
				next_state<=go_ep02;
			end if;

		when go_ep82 =>					--start transfer to EP82
			if fsm_rdy='0' then
				next_state<=check_priority;
			else
				next_state<=go_ep82;
			end if;

		when go_ep83 =>					--start transfer from EP83
			if fsm_rdy='0' then
				next_state<=check_priority;
			else
				next_state<=go_ep83;
			end if;


		when others =>
>>>>>>> Revert "enlever le chain de argu"
			next_state<=idle;
	end case;
end process;





<<<<<<< refs/remotes/upstream/main
  
end arch;





=======

end arch;
>>>>>>> Revert "enlever le chain de argu"
