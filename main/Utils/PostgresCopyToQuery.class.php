<?php
	/**
	 * @author Timofey Anisimov <t.anisimov@co.wapstart.ru>
	 * @copyright Copyright (c) 2011, Wapstart
	 */
	final class PostgresCopyToQuery
	{
		private $tableName = null;
		private $db = null;
		private $delimiter = ',';

		public static function create(DB $db)
		{
			return new self($db);
		}

		public function __construct(DB $db)
		{
			$this->db = $db;
		}

		public function setTableName($tableName)
		{
			$this->tableName = $tableName;

			return $this;
		}

		public function setDelimeter($delimeter)
		{
			$this->delimiter = $delimeter;

			return $this;
		}

		public function run()
		{
			return
				pg_copy_to(
					$this->db->getLink(),
					$this->tableName,
					$this->delimiter
				);
		}
	}
?>
