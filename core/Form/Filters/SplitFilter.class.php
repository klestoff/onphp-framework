<?php
	/**
	 * @author Alexander Klestov <a.klestov@co.wapstart.ru>
	 * @copyright Copyright (c) 2011, Wapstart
	 */
	final class SplitFilter implements Filtrator
	{
		private $separator		= ' ';
		private $splitByRegExp	= false;
		
		/**
		 * @return SplitFilter
		 */
		public static function create() 
		{
			return new self();
		}
		
		public function getSeparator() 
		{
			return $this->separator;
		}

		/**
		 * @return SplitFilter 
		 */
		public function setSeparator($separator) 
		{
			$this->separator = $separator;
			
			return $this;
		}
		
		public function isSplitByRegExp() 
		{
			return $this->splitByRegExp;
		}

		public function setSplitByRegExp($splitByRegExp = true) 
		{
			$this->splitByRegExp = ($splitByRegExp === true);
		}

		public function apply($value) 
		{
			return 
				$this->isSplitByRegExp()
					? 
						preg_split(
							$this->separator,
							$this->value,
							-1,
							PREG_SPLIT_NO_EMPTY
						)
					: explode($this->separator, $value);
		}

	}
?>
